#ifndef CAMERA_H
#define CAMERA_H


#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

//#include <stdio.h>

#include <vector>



// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	DOWN,
	UP,
	ACCELERATION,
	DECELERATION,
	GRAVITY
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 1.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;
//const float accelaration = 5.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 LastPosition = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	glm::vec3 Target;
	float accelaration = 5.0f;
	// Euler Angles
	float Yaw;
	float Pitch;
	float Xrotation;
	float Yrotation;
	float Zrotation;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity = 0.0f;
	float Zoom;
	// Camera Player
	bool onPlayer;
	//freeze
	bool freeze, saveMovement, restoreMovement;
	glm::vec4 pwr;
	//projection
	glm::mat4 projection;
	//depth buffer id
	GLuint depthMapFBO;
	GLuint depthMapTexture;

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
		onPlayer = false;
		freeze = false;
		saveMovement = true;
		restoreMovement = false;
		Target = glm::vec3(0.0);
		Xrotation = 0.0f;
		Yrotation = 0.0f;
		Zrotation = 0.0f;
	}
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, glm::vec3(0.0), Up);
	}

	//setter of projection
	void setProjection(glm::mat4 projection)
	{
		this->projection = projection;
	}

	//getter of projection
	glm::mat4 getProjection()
	{
		return this->projection;
	}

	//create fbo
    void configure_depthMap(int screenWidth = 1024, int screenHeight = 1024) {
        // configure depth map FBO
        // -----------------------
        glGenFramebuffers(1, &depthMapFBO);
        // create depth texture
        glGenTextures(1, &depthMapTexture);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		if (!freeze) {
			float velocity = MovementSpeed * deltaTime * accelaration;
			if (direction == FORWARD)
				Position += Front * velocity;
			if (direction == BACKWARD)
				Position -= Front * velocity;
			if (direction == LEFT)
				Position -= Right * velocity;
			if (direction == RIGHT)
				Position += Right * velocity;
			if (direction == UP)
				Position += WorldUp * velocity;
			if (direction == DOWN)
				Position -= WorldUp * velocity;
			/*if (direction == ACCELERATION && velocity < 1)
				accelaration += 0.05;
			if (direction == DECELERATION && velocity > 0.05)
				accelaration -= 0.05;*/
			if (direction == GRAVITY)
				Position -= WorldUp * velocity;
		}
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		if (!freeze)
		{
			if (LastPosition.y < Position.y) {
				//	std::cout << "salut" << std::endl;
			}
			xoffset *= MouseSensitivity;
			yoffset *= MouseSensitivity;

			Yaw += xoffset;
			Pitch += yoffset;
			if (constrainPitch)
			{
				if (Pitch > 89.9f)
					Pitch = 89.9f;
				if (Pitch < -89.9f)
					Pitch = -89.9f;
			}
			if (!onPlayer) {


				// Make sure that when pitch is out of bounds, screen doesn't get flipped


				// Update Front, Right and Up Vectors using the updated Euler angles
				updateCameraVectors();
			}
			else {
				/*xoffset *= MouseSensitivity;
				yoffset *= MouseSensitivity;*/

				Xrotation += xoffset;
				Yrotation += yoffset;

				if (Yrotation > 179)Yrotation = 179;
				if (Yrotation < 1)Yrotation = 1;

			}
			saveMovement = true;
		}
		if (freeze && saveMovement)
		{
			pwr = glm::vec4(Pitch, Yaw, Xrotation, Yrotation);
			saveMovement = false;
		}
		if (restoreMovement)
		{
			Pitch = pwr.x;
			Yaw = pwr.y;
			Xrotation = pwr.z;
			Yrotation = pwr.w;
			restoreMovement = false;
		}
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

	//invert the pitch
	void invertPitch()
	{
		Pitch = -Pitch;
		updateCameraVectors();
	}

	void addYDistance(float distance)
	{
		Position.y += distance;
	}

	void updateCameraVectorsFromFront()
	{

		//Pitch = glm::degrees(asin(Front.y));
		//Yaw = glm::degrees(acos(Front.x / cos(glm::radians(Pitch))));
		Front = glm::normalize(Front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}

	void lookAt(glm::vec3 target) {
		if (onPlayer) {
			Target = target;
			Target.y += 1.0f;
			Front = Target - Position;
			updateCameraVectorsFromFront();


			Position = Target + glm::vec3(cos(glm::radians(Xrotation))*10.0f, 10.0f + 10.0f*cos(glm::radians(Yrotation)), sin(glm::radians(Xrotation))*10.0f);
		}
	}
private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}


};
#endif
