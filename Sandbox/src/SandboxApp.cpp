#include <Hazel.h>
#include "Platform/OpenGL/OpenGLShader.h"

#include <memory>
#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
	{
		m_TriangleVA.reset(Hazel::VertexArray::Create());

		float vertices[3 * 7] = {
			-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
		};

		Hazel::Ref<Hazel::VertexBuffer> triangleVB;
		triangleVB.reset(Hazel::VertexBuffer::Create(vertices, sizeof(vertices)));
		triangleVB->SetLayout({
			{ Hazel::ShaderDataType::Float3, "a_Position" },
			{ Hazel::ShaderDataType::Float4,  "a_Color" }
			});
		m_TriangleVA->AddVertexBuffer(triangleVB);

		unsigned int indices[3] = { 0, 1, 2 };

		Hazel::Ref<Hazel::IndexBuffer> triangleIB;
		triangleIB.reset(Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
		m_TriangleVA->SetIndexBuffer(triangleIB);

		std::string vertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string fragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

		m_ShaderLibrary.Add(Hazel::Shader::Create("VertexPosColor", vertexSrc, fragmentSrc));

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
		};

		m_SquareVA.reset(Hazel::VertexArray::Create());

		Hazel::Ref<Hazel::VertexBuffer> squareVB;
		squareVB.reset(Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ Hazel::ShaderDataType::Float3, "a_Position" },
			{ Hazel::ShaderDataType::Float2, "a_TextCoord" }
			});
		m_SquareVA->AddVertexBuffer(squareVB);

		unsigned int squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Hazel::Ref<Hazel::IndexBuffer> squareIB;
		squareIB.reset(Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string flatColorShaderVertexSrc = R"(
			#version 330 core

			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			void main()
			{
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColorShaderfragmentSrc = R"(
			#version 330 core

			layout(location = 0) out vec4 color;

			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0);
			}
		)";

		m_ShaderLibrary.Add(Hazel::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderfragmentSrc));

		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = Hazel::Texture2D::Create("assets/textures/checkerboard.png");
		m_LogoTexture = Hazel::Texture2D::Create("assets/textures/logo.png");

		std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Hazel::Timestep ts) override
	{
		if (Hazel::Input::IsKeyPressed(HZ_KEY_A))
			m_CameraPosition.x -= m_CameraSpeed * ts;
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_D))
			m_CameraPosition.x += m_CameraSpeed * ts;

		if (Hazel::Input::IsKeyPressed(HZ_KEY_W))
			m_CameraPosition.y += m_CameraSpeed * ts;
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_S))
			m_CameraPosition.y -= m_CameraSpeed * ts;

		if (Hazel::Input::IsKeyPressed(HZ_KEY_E))
			m_CameraRotation -= m_CameraRotationSpeed * ts;
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_Q))
			m_CameraRotation += m_CameraRotationSpeed * ts;

		Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Hazel::RenderCommand::ClearColor();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Hazel::Renderer::BeginScene(m_Camera);

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		glm::vec4 redColor(0.8f, 0.3f, 0.8f, 1.0f);
		glm::vec4 blueColor(0.2f, 0.3f, 0.8f, 1.0f);

		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_ShaderLibrary.Get("FlatColor"))->Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_ShaderLibrary.Get("FlatColor"))->UploadUniformFloat3("u_Color", m_SquareColor);

		for (int i = 0; i < 20; i++)
		{
			for (int j = 0; j < 20; j++)
			{
				glm::vec3 pos(i * 0.11f, j * 0.22f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Hazel::Renderer::Submit(m_ShaderLibrary.Get("FlatColor"), m_SquareVA, transform);
			}
		}

		auto textureShader = m_ShaderLibrary.Get("Texture");
		m_Texture->Bind();
		Hazel::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
		m_LogoTexture->Bind();
		Hazel::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
		//Hazel::Renderer::Submit(m_Shader, m_TriangleVA);

		Hazel::Renderer::EndScene();
	}

	void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}
private:
	Hazel::ShaderLibrary m_ShaderLibrary;
	Hazel::Ref<Hazel::VertexArray> m_TriangleVA;
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;
	Hazel::Ref<Hazel::Texture2D> m_Texture, m_LogoTexture;

	Hazel::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
	float m_CameraRotation = 0.0f;
	float m_CameraSpeed = 1.0f;
	float m_CameraRotationSpeed = 20.0f;

	glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}
