#pragma once

#include <glm/glm.hpp>

#include <glm/gtx/matrix_decompose.hpp>

namespace MathUtil {
	//TODO: Put in some sort of Math::Util folder
	static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale) {
		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		//Normalize the Matrix
		if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
			return false;

		//Isolate Perspective
		if (
			epsilonEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>())
			) {
			//Clear Perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		//Translation
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3], Pdum3;

		//Scale and shear
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		//Compute X scale factor and normalize first row
		scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

		//Rotation
		rotation.y = asin(-Row[0][2]);
		if (cos(rotation.y) != 0) {
			rotation.x = atan2(Row[1][2], Row[2][2]);
			rotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else {
			rotation.x = atan2(-Row[2][0], Row[1][1]);
			rotation.z = 0;
		}

		return true;
	}

}