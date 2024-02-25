#include "Camera.hpp"

namespace gps {

	//Camera constructor
	Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
		//TODO
		this->cameraPosition = cameraPosition;
		this->cameraTarget = cameraTarget;

		this->cameraUpDirection = cameraUp;
		this->cameraFrontDirection = cameraPosition - cameraTarget;
		this->cameraRightDirection = glm::cross(this->cameraUpDirection, this->cameraFrontDirection);

	}

	//return the view matrix, using the glm::lookAt() function
	glm::mat4 Camera::getViewMatrix() {
		return glm::lookAt(cameraPosition, cameraTarget, 
			cameraUpDirection);
	}
	//update the camera internal parameters following a camera move event
	void Camera::move(MOVE_DIRECTION direction, float speed) {
		//TODO
		glm::vec3 dir;
		//toate posibilitatile de miscare
		if (direction == MOVE_FORWARD) {
			dir = this->cameraFrontDirection * (-speed);
		}
		else if (direction == MOVE_BACKWARD) {
			dir = this->cameraFrontDirection * speed;
		}
		else if (direction == MOVE_RIGHT) {
			dir = this->cameraRightDirection * speed;
		}
		else if (direction == MOVE_LEFT) {
			dir = this->cameraRightDirection * (-speed);
		}
		else if (direction == MOVE_UP) {
			dir = this->cameraUpDirection * speed;
		}
		else if (direction == MOVE_DOWN) {
			dir = this->cameraUpDirection * (-speed);
		}
		glm::mat4 model = glm::mat4(1.0f);
		this->cameraPosition += dir;
		this->cameraTarget += dir;
	}

	//update the camera internal parameters following a camera rotate event
	//yaw - camera rotation around the y axis
	//pitch - camera rotation around the x axis

	void Camera::rotate(float pitch, float yaw) {
		//TODO
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, this->cameraPosition);
		model = glm::rotate(model, yaw, this->cameraUpDirection);
		model = glm::rotate(model, pitch, this->cameraRightDirection);
		model = glm::translate(model, -this->cameraPosition);

		glm::vec4 temp = model * glm::vec4(this->cameraTarget, 1.0f);

		this->cameraTarget = temp;
		this->cameraFrontDirection = this->cameraPosition - this->cameraTarget;
		this->cameraRightDirection = glm::cross(this->cameraUpDirection, this->cameraFrontDirection);
	}
}