#include <glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<stdexcept>
#include<iostream>
#include<cmath>
#include<vector>
#include<string>
#include "text_render.hpp"

#define radius 0.6f
//callback function to adjust the viewport when the window size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
//process all input and can be added more features later
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
//function to generate circle vertices
std::vector<float> generateVerices_rgb_Circle(int num) {
	std::vector<float> vertices;
	vertices.push_back(0.0f); // Center vertex x
	vertices.push_back(0.0f); // Center vertex y
	vertices.push_back(0.0f); // Center vertex z
	for(int i=0; i <= num; ++i) {
		float angle = 2.0f * 3.1415926f * float(i) / float(num);
		float x = radius*cosf(angle);
		vertices.push_back(x);
		float y = radius*sinf(angle);
		vertices.push_back(y);
		float z = 0.0f;
		vertices.push_back(z);
		// Store vertex positions
	}
	return vertices;
}
//function to check if the point is inside the circle
bool is_inside_circle(float x, float y) {
	return (x * x + y * y) <= (radius * radius);
}
//function to check if the point is inside the alpha box
bool is_inside_alpha_box(float x, float y) {
	return (x >= 0.8f && x <= 0.9f &&
		y >= -0.8f && y <= 0.8f);
}
//function to convert screen coordinates to NDC
void screenToNDC(GLFWwindow* window, double xpos, double ypos, float& ndcx, float& ndcy) {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	ndcx = static_cast<float>((xpos / width) * 2.0 - 1.0);
	ndcy = static_cast<float>(1.0 - (ypos / height) * 2.0);
}
//a global variable to store the triangle y offset
float triangleYoffset = 0.0f;
//function to calculate alpha value based on y position
float calculateAlphaValue(float ypos) {
	if (ypos < -0.8f) {
		return 0.0f;
	}
	if (ypos > 0.8f) {
		return 1.0f;
	}
	float a = (ypos + 0.8f) / 1.6f;
	return a;
}
//a global variable to store the final color
glm::vec4 finalColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
//HSV to RGB conversion function
glm::vec3 HSVtoRGB(float H, float S, float V) {
	float C = V * S;
	float X = C * (1.0f - fabsf(fmodf(H / 60.0f, 2.0f) - 1.0f));
	float m = V - C;
	float r, g, b;
	if (H >= 0 && H < 60) {
		r = C; g = X; b = 0;
	}
	else if (H >= 60 && H < 120) {
		r = X; g = C; b = 0;
	}
	else if (H >= 120 && H < 180) {
		r = 0; g = C; b = X;
	}
	else if (H >= 180 && H < 240) {
		r = 0; g = X; b = C;
	}
	else if (H >= 240 && H < 300) {
		r = X; g = 0; b = C;
	}
	else {
		r = C; g = 0; b = X;
	}
	return glm::vec3(r + m, g + m, b + m);
}

//call back function
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double sx, sy;
		glfwGetCursorPos(window, &sx, &sy);
		float x, y;
		screenToNDC(window, sx, sy, x, y);
		if (is_inside_alpha_box(x, y)) {
			//std::cout << "Rectangle clicked!" << std::endl;
			triangleYoffset = y;
			float alphaValue = calculateAlphaValue(y);
			finalColor.a = alphaValue;
			//std::cout << "Alpha Value: " << alphaValue << std::endl;
		}
		if (is_inside_circle(x, y)) {
			//std::cout << "Circle clicked!" << std::endl;
			float angle = atan2(y, x);
			float hue = (angle + 3.1415926f) / (2.0f * 3.1415926f);
			float in_radius = sqrt(x * x + y * y);
			float saturation = in_radius / radius;
			saturation = std::min(std::max(saturation, 0.0f), 1.0f);
			float value = 1.0f;
			glm::vec3 rgb = HSVtoRGB(hue * 360.0f, saturation, value);
			finalColor.r = rgb.r;
			finalColor.g = rgb.g;
			finalColor.b = rgb.b;
		}
	}
}
//shader sources
const char* circle_vs_shader = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"out vec3 vPos;\n"
"void main()\n"
"{\n"
"    vPos = aPos;\n"
"    gl_Position = vec4(aPos, 1.0);\n"
"}\n";

const char* circle_fs_shader = "#version 330 core\n"
"in vec3 vPos;\n"
"out vec4 FragColor;\n"
"vec3 hsv2rgb(vec3 c)\n"
"{\n"
"    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
"    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
"    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
"}\n"
"void main()\n"
"{\n"
"    float radius = length(vPos.xy);\n"
"	if (radius>0.6)discard;\n"
"    float angle = atan(vPos.y, vPos.x);\n"
"    float hue = (angle + 3.1415926) / (2.0 * 3.1415926);\n"
"    float saturation = clamp(radius/0.6f,0.0f,1.0f);\n"
"    float value = 1.0;\n"
"    vec3 rgb = hsv2rgb(vec3(hue, saturation, value));\n"
"    FragColor = vec4(rgb, 1.0);\n"
"}\n";

const char* ui_vs_shader = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aColor;\n"
"uniform float offSetY;\n"
"out vec3 vColor;\n"
"void main()\n"
"{\n"
"    vColor = aColor;\n"
"    vec3 pos = aPos;\n"
"    pos.y += offSetY;\n"
"    gl_Position = vec4(pos, 1.0);\n"
"}\n";

const char* ui_fs_shader = "#version 330 core\n"
"in vec3 vColor;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"	
"    FragColor = vec4(vColor, 1.0f);\n"
"}\n";

const char* final_box_vs_shader = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(aPos, 1.0);\n"
"}\n";

const char* final_box_fs_shader = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 uColor;\n"
"void main()\n"
"{\n"
"FragColor = uColor;\n"
"}\n";	

int main() {
	std::cout << "A brief description of the project: \n";
	std::cout << "A fully GPU-driven RGB color picker built using modern OpenGL, implementing an HSV color wheel, interactive alpha adjustment, and real-time RGBA visualization. \nThe project has shader programming, custom UI rendering, mouse input processing, coordinate transformations, alpha blending, and font rendering, all without relying on external UI libraries";
	
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialize GLFW terminating it!");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(800, 800, "RGB circle", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window");
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
		glfwTerminate();
		throw std::runtime_error("Failed to initialize GLAD");
	}
	// enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// generate circle vertices
	std::vector<float> vertices=generateVerices_rgb_Circle(360);
	// set up circle graphics pipeline
	unsigned int circle_VBO, circle_VAO;
	glGenVertexArrays(1, &circle_VAO);
	glGenBuffers(1, &circle_VBO);
	glBindVertexArray(circle_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, circle_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	unsigned int circle_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(circle_vertexShader, 1, &circle_vs_shader, NULL);
	glCompileShader(circle_vertexShader);
	int success;
	char infoLog[1024];
	glGetShaderiv(circle_vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(circle_vertexShader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::CIRCLE_VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	unsigned int circle_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(circle_fragmentShader, 1, &circle_fs_shader, NULL);
	glCompileShader(circle_fragmentShader);
	glGetShaderiv(circle_fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(circle_fragmentShader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::CIRCLE_FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	unsigned int circle_shaderProgram = glCreateProgram();
	glAttachShader(circle_shaderProgram, circle_vertexShader);
	glAttachShader(circle_shaderProgram, circle_fragmentShader);
	glLinkProgram(circle_shaderProgram);
	glGetProgramiv(circle_shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(circle_shaderProgram, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::CIRCLE_PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(circle_vertexShader);
	glDeleteShader(circle_fragmentShader);
	// set up alpha box graphics pipeline
	std::vector<float> alpha_box_vertices = {
		0.8f, 0.8f, 0.0f, 1.0f,1.0f,1.0f,
		0.8f, -0.8f, 0.0f, 0.0f,0.0f,0.0f,
		0.9f, -0.8f, 0.0f, 0.0f,0.0f,0.0f,
		0.9f, 0.8f, 0.0f, 1.0f,1.0f,1.0f,
	};
	std::vector<unsigned int>indices={
		0,1,2,
		2,3,0
	};
	unsigned int alpha_box_VBO,alpha_box_VAO;
	glGenVertexArrays(1, &alpha_box_VAO);
	glGenBuffers(1, &alpha_box_VBO);
	glBindVertexArray(alpha_box_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, alpha_box_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * alpha_box_vertices.size(), alpha_box_vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	unsigned int alpha_box_EBO;
	glGenBuffers(1, &alpha_box_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, alpha_box_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);
	unsigned int alpha_box_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(alpha_box_vertexShader, 1, &ui_vs_shader, NULL);
	glCompileShader(alpha_box_vertexShader);
	glGetShaderiv(alpha_box_vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(alpha_box_vertexShader, 1024, NULL, infoLog);
		std::cout << "ERROR::ALpha_box_SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	unsigned int alpha_box_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(alpha_box_fragmentShader, 1, &ui_fs_shader, NULL);
	glCompileShader(alpha_box_fragmentShader);
	glGetShaderiv(alpha_box_fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(alpha_box_fragmentShader, 1024, NULL, infoLog);
		std::cout << "ERROR::ALPHA_BOX_SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	unsigned int ui_shaderProgram = glCreateProgram();
	glAttachShader(ui_shaderProgram, alpha_box_vertexShader);
	glAttachShader(ui_shaderProgram, alpha_box_fragmentShader);
	glLinkProgram(ui_shaderProgram);
	glGetProgramiv(ui_shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ui_shaderProgram, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::ALPHA_BOX_PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	// set up alpha triangle graphics pipeline
	std::vector<float>alpha_triangle = {
		0.75f, 0.8f, 0.0f,  137.0f / 255.0f, 137.0f / 255.0f, 137.0f / 255.0f,
		0.75f, 0.7f, 0.0f,  137.0f / 255.0f, 137.0f / 255.0f, 137.0f / 255.0f,
		0.8f, 0.75f, 0.0f,   137.0f / 255.0f, 137.0f / 255.0f, 137.0f / 255.0f
	};
	unsigned int alpha_triangle_VBO, alpha_triangle_VAO;
	glGenVertexArrays(1, &alpha_triangle_VAO);
	glGenBuffers(1, &alpha_triangle_VBO);
	glBindVertexArray(alpha_triangle_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, alpha_triangle_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * alpha_triangle.size(), alpha_triangle.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// set up final output box graphics pipeline
	std::vector<float> output_box_vertices = {
		-0.5f, -0.8f, 0.0f,
		-0.5f, -0.9f, 0.0f,
		 0.5f, -0.9f, 0.0f,
		 0.5f, -0.8f, 0.0f,
	};
	std::vector<unsigned int>output_indices = {
		0,1,2,
		2,3,0
	};
	unsigned int output_box_VBO, output_box_VAO;
	glGenVertexArrays(1, &output_box_VAO);
	glGenBuffers(1, &output_box_VBO);
	glBindVertexArray(output_box_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, output_box_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * output_box_vertices.size(), output_box_vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	unsigned int output_box_EBO;
	glGenBuffers(1, &output_box_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, output_box_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* output_indices.size(), output_indices.data(), GL_STATIC_DRAW);
	unsigned int output_box_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(output_box_vertexShader, 1, &final_box_vs_shader, NULL);
	glCompileShader(output_box_vertexShader);
	glGetShaderiv(output_box_vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(output_box_vertexShader, 1024, NULL, infoLog);
		std::cout << "ERROR::OUTPUT_BOX_SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	unsigned int output_box_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(output_box_fragmentShader, 1, &final_box_fs_shader, NULL);
	glCompileShader(output_box_fragmentShader);
	glGetShaderiv(output_box_fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(output_box_fragmentShader, 1024, NULL, infoLog);
		std::cout << "ERROR::OUTPUT_BOX_SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	unsigned int output_box_shaderProgram = glCreateProgram();
	glAttachShader(output_box_shaderProgram, output_box_vertexShader);
	glAttachShader(output_box_shaderProgram, output_box_fragmentShader);
	glLinkProgram(output_box_shaderProgram);
	glGetProgramiv(output_box_shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(output_box_shaderProgram, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::OUTPUT_BOX_PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	// clean up shaders
	glDeleteShader(alpha_box_vertexShader);
	glDeleteShader(alpha_box_fragmentShader);
	glDeleteShader(output_box_vertexShader);
	glDeleteShader(output_box_fragmentShader);
	// initialize text rendering
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	initText(width,height) ;

	// render
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		// draw circle
		glUseProgram(circle_shaderProgram);
		glBindVertexArray(circle_VAO);          
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size() / 3);

		// draw alpha box
		glUseProgram(ui_shaderProgram);
		GLuint offSetYLocation = glGetUniformLocation(ui_shaderProgram, "offSetY");
		glUniform1f(offSetYLocation, 0.0f);
		
		glBindVertexArray(alpha_box_VAO);       
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

		// draw alpha triangle
		glUniform1f(offSetYLocation, triangleYoffset-0.75f);
		glBindVertexArray(alpha_triangle_VAO);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(alpha_triangle.size() / 6));

		// draw output box
		glUseProgram(output_box_shaderProgram);
		glUniform4f(
			glGetUniformLocation(output_box_shaderProgram, "uColor"),
			finalColor.r, finalColor.g, finalColor.b, finalColor.a
		);

		glBindVertexArray(output_box_VAO);
		glDrawElements(GL_TRIANGLES, output_indices.size(), GL_UNSIGNED_INT, 0);
		// render text
		renderText("R:"+std::to_string(finalColor.r), 20, 20, 1.0f, 1.0f, 1.0f, 1.0f);
		renderText("G:"+std::to_string(finalColor.g), 20, 40, 1.0f, 1.0f, 1.0f, 1.0f);
		renderText("B:" + std::to_string(finalColor.b), 20, 60, 1.0f, 1.0f, 1.0f, 1.0f);
		renderText("A:" + std::to_string(finalColor.a), 20, 80, 1.0f, 1.0f, 1.0f, 1.0f);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	// Free memory
	std::vector<float>().swap(vertices); 
	std::vector<float>().swap(alpha_box_vertices); 
	std::vector<float>().swap(alpha_triangle); 
	std::vector<float>().swap(output_box_vertices);
	return 0;
}