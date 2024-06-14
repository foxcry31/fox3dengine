#include "mesh.h"

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, std::vector<Texture>& textures)
{
    Mesh::vertices = vertices;
    Mesh::indices = indices;
    Mesh::textures = textures;

    VAO vao;
	vao.Bind();

	VBO vbo(vertices);
	EBO ebo(indices);

	vao.LinkAttributes(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	vao.LinkAttributes(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	vao.LinkAttributes(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	vao.LinkAttributes(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));

	vao.Unbind();
	vbo.Unbind();
	ebo.Unbind();
}

void Mesh::Draw(Shader& shader, Camera& camera)
{
    shader.Activate();
    vao.Bind();

    unsigned int numberDiffuse = 0;
    unsigned int numberSpecular = 0;

    for (unsigned int i = 0; i < textures.size(); i++)
    {
        std::string num;
        std::string type = textures[i].type;
        if (type == "diffuse")
        {
            num = std::to_string(numberDiffuse++);
        }
        else if (type == "specular")
        {
            num = std::to_string(numberSpecular++);
        }
        textures[i].texUnit(shader, (type + num).c_str(), i);
        textures[i].Bind();
    }
    glUniform3f(glGetUniformLocation(shader.id, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
    camera.Matrix(shader, "camera");

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
