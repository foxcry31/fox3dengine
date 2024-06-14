#include "model.h"

Model::Model(const char* file)
{
    std::string text = get_file_contents(file);
    JSON = json::parse(text);

    Model::file = file;
    data = getData();

    traverseNode(0);
}

void Model::Draw(Shader& shader, Camera& camera)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].Mesh::Draw(shader, camera, matricesMeshes[i]);
    }
}


void Model::loadMesh(unsigned int indexMesh)
{
    unsigned int positionAccessorIndex = JSON["meshes"][indexMesh]["primitives"][0]["attributes"]["POSITION"];
    unsigned int normalAccessorIndex = JSON["meshes"][indexMesh]["primitives"][0]["attributes"]["NORMAL"];
    unsigned int textureAccessorIndex = JSON["meshes"][indexMesh]["primitives"][0]["attributes"]["TEXCOORD_0"];
    unsigned int indicesAccessorIndex = JSON["meshes"][indexMesh]["primitives"][0]["indices"];

    std::vector<float> positionVector = getFloats(JSON["accessors"][positionAccessorIndex]);
    std::vector<glm::vec3> positions = groupFloatsVec3(positionVector);
    std::vector<float> normalVector = getFloats(JSON["accessors"][normalAccessorIndex]);
    std::vector<glm::vec3> normals = groupFloatsVec3(normalVector);
    std::vector<float> textureVector = getFloats(JSON["accessors"][textureAccessorIndex]);
    std::vector<glm::vec2> textureUVs = groupFloatsVec2(textureVector);

    std::vector<Vertex> vertices = assembleVertices(positions, normals, textureUVs);
    std::vector<GLuint> indices = getIndices(JSON["accessors"][indicesAccessorIndex]);
    std::vector<Texture> textures = getTextures();

    meshes.push_back(Mesh(vertices, indices, textures));
}

void Model::traverseNode(unsigned int nextNode, glm::mat4 matrix)
{
	json node = JSON["nodes"][nextNode];

	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
	if (node.find("translation") != node.end())
	{
		float transValues[3];
		for (unsigned int i = 0; i < node["translation"].size(); i++)
			transValues[i] = (node["translation"][i]);
		translation = glm::make_vec3(transValues);
	}

	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	if (node.find("rotation") != node.end())
	{
		float rotValues[4] =
		{
			node["rotation"][3],
			node["rotation"][0],
			node["rotation"][1],
			node["rotation"][2]
		};
		rotation = glm::make_quat(rotValues);
	}

	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	if (node.find("scale") != node.end())
	{
		float scaleValues[3];
		for (unsigned int i = 0; i < node["scale"].size(); i++)
			scaleValues[i] = (node["scale"][i]);
		scale = glm::make_vec3(scaleValues);
	}

	glm::mat4 matNode = glm::mat4(1.0f);
	if (node.find("matrix") != node.end())
	{
		float matValues[16];
		for (unsigned int i = 0; i < node["matrix"].size(); i++)
			matValues[i] = (node["matrix"][i]);
		matNode = glm::make_mat4(matValues);
	}

	glm::mat4 trans = glm::mat4(1.0f);
	glm::mat4 rot = glm::mat4(1.0f);
	glm::mat4 sca = glm::mat4(1.0f);

	trans = glm::translate(trans, translation);
	rot = glm::mat4_cast(rotation);
	sca = glm::scale(sca, scale);

	glm::mat4 matNextNode = matrix * matNode * trans * rot * sca;

	if (node.find("mesh") != node.end())
	{
		translationsMeshes.push_back(translation);
		rotationsMeshes.push_back(rotation);
		scalesMeshes.push_back(scale);
		matricesMeshes.push_back(matNextNode);

		loadMesh(node["mesh"]);
	}

	if (node.find("children") != node.end())
	{
		for (unsigned int i = 0; i < node["children"].size(); i++)
			traverseNode(node["children"][i], matNextNode);
	}
}

std::vector<unsigned char> Model::getData()
{
    std::string bytesText;
    std::string uri = JSON["buffers"][0]["uri"];

    std::cout << uri << std::endl;

    std::string fileString = std::string(file);
    std::string fileDirectory = fileString.substr(0, fileString.find_last_of('/') + 1);

    std::cout << fileString << std::endl;
    std::cout << fileDirectory << std::endl;

    bytesText = get_file_contents((fileDirectory + uri).c_str());

    std::vector<unsigned char> data(bytesText.begin(), bytesText.end());
    return data;
}

std::vector<float> Model::getFloats(json accessor)
{
    std::vector<float> floatVector;

    unsigned int bufferViewIndex = accessor.value("bufferView", 1);
    unsigned int count = accessor["count"];
    unsigned int accessorByteOffset = accessor.value("byteOffset", 0);
    std::string type = accessor["type"];

    json bufferView = JSON["bufferViews"][bufferViewIndex];
    unsigned int byteOffset = bufferView["byteOffset"];

    unsigned int numberPerVertex;
    if (type == "SCALAR") numberPerVertex = 1;
    else if (type == "VEC2") numberPerVertex = 2;
    else if (type == "VEC3") numberPerVertex = 3;
    else if (type == "VEC4") numberPerVertex = 4;
    else throw std::invalid_argument("Type is invaild (not SCALAR, VEC2, VEC3 or VEC4)");

    unsigned int beginningOfData = byteOffset + accessorByteOffset;
    unsigned int lengthOfData = count * 4 * numberPerVertex;
    for (unsigned int i = beginningOfData; i < beginningOfData + lengthOfData; i)
    {
        unsigned char bytes[] = {data[i++], data[i++], data[i++], data[i++]};
        float value;
        std::memcpy(&value, bytes, sizeof(float));
        floatVector.push_back(value);
    }

    return floatVector;
}

std::vector<GLuint> Model::getIndices(json accessor)
{
    std::vector<GLuint> indices;

    unsigned int bufferViewIndex = accessor.value("bufferView", 0);
    unsigned int count = accessor["count"];
    unsigned int accessorByteOffset = accessor.value("byteOffset", 0);
    unsigned int componentType = accessor["componentType"];

    json bufferView = JSON["bufferViews"][bufferViewIndex];
    unsigned int byteOffset = bufferView["byteOffset"];

    unsigned int beginningOfData = byteOffset + accessorByteOffset;
    if (componentType == 5125)
    {
        for (unsigned int i = beginningOfData; i < beginningOfData + count * 4; i)
        {
            unsigned char bytes[] = {data[i++], data[i++], data[i++], data[i++]};
            unsigned int value;
            std::memcpy(&value, bytes, sizeof(unsigned int));
            indices.push_back((GLuint)value);
        }
    }
    else if (componentType == 5123)
    {
        for (unsigned int i = beginningOfData; i < beginningOfData + count * 2; i)
        {
            unsigned char bytes[] = {data[i++], data[i++]};
            unsigned short value;
            std::memcpy(&value, bytes, sizeof(unsigned short));
            indices.push_back((GLuint)value);
        }
    }
    else if (componentType == 5122)
    {
        for (unsigned int i = beginningOfData; i < byteOffset + accessorByteOffset + count * 2; i)
        {
            unsigned char bytes[] = {data[i++], data[i++]};
            short value;
            std::memcpy(&value, bytes, sizeof(short));
            indices.push_back((GLuint)value);
        }
    }

    return indices;
}

std::vector<Texture> Model::getTextures()
{
    std::vector<Texture> textures;

    std::string fileString = std::string(file);
    std::string fileDirectory = fileString.substr(0, fileString.find_last_of('/') + 1);

    for (unsigned int i = 0; i < JSON["images"].size(); i++)
    {
        std::string texturePath = JSON["images"][i]["uri"];

        bool skip = false;
        for (unsigned int j = 0; j < loadedTextureName.size(); j++)
        {
            if (loadedTextureName[j] == texturePath)
            {
                textures.push_back(loadedTexture[j]);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            if (texturePath.find("baseColor") != std::string::npos)
            {
                Texture diffuse = Texture((fileDirectory + texturePath).c_str(), "diffuse", loadedTexture.size());
                textures.push_back(diffuse);
                loadedTexture.push_back(diffuse);
                loadedTextureName.push_back(texturePath);
            }
            else if (texturePath.find("metallicRoughness") != std::string::npos)
            {
                Texture specular = Texture((fileDirectory + texturePath).c_str(), "specular", loadedTexture.size());
                textures.push_back(specular);
                loadedTexture.push_back(specular);
                loadedTextureName.push_back(texturePath);
            }
        }
    }
    return textures;
}


std::vector<Vertex> Model::assembleVertices(std::vector<glm::vec3> positions, std::vector<glm::vec3> normals, std::vector<glm::vec2> textureUVs)
{
    std::vector<Vertex> vertices;
    for (int i = 0; i < positions.size(); i++)
    {
        vertices.push_back(Vertex{positions[i], normals[i], glm::vec3(1.0f, 1.0f, 1.0f), textureUVs[i]});
    }
    return vertices;
}


std::vector<glm::vec2> Model::groupFloatsVec2(std::vector<float> floatVector)
{
    std::vector<glm::vec2> vectors;
    for (int i = 0; i < floatVector.size(); i)
    {
        vectors.push_back(glm::vec2(floatVector[i++], floatVector[i++]));
    }
    return vectors;
}

std::vector<glm::vec3> Model::groupFloatsVec3(std::vector<float> floatVector)
{
    std::vector<glm::vec3> vectors;
    for (int i = 0; i < floatVector.size(); i)
    {
        vectors.push_back(glm::vec3(floatVector[i++], floatVector[i++], floatVector[i++]));
    }
    return vectors;
}

std::vector<glm::vec4> Model::groupFloatsVec4(std::vector<float> floatVector)
{
    std::vector<glm::vec4> vectors;
    for (int i = 0; i < floatVector.size(); i)
    {
        vectors.push_back(glm::vec4(floatVector[i++], floatVector[i++], floatVector[i++], floatVector[i++]));
    }
    return vectors;
}

