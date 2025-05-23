﻿#pragma once

#include <PaperEngine/core/Base.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace PaperEngine {

	class Transform {
	public:

		Transform() = default;
		Transform(const Transform&) = default;
		Transform& operator=(const Transform&) = default;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="axis"></param>
		/// <param name="angle">
		/// in degree
		/// </param>
		/// <returns></returns>
		PE_API void rotate(const glm::vec3& axis, float angle);

		PE_API void set_position(const glm::vec3& position) { m_isDirty = true; m_position = position; }

		PE_API void set_rotation(const glm::quat& rotation) { m_isDirty = true; m_rotation = rotation; }

		PE_API void set_scale(const glm::vec3& scale) { m_isDirty = true; m_scale = scale; }

		PE_API const glm::vec3& get_position() const { return m_position; }

		PE_API const glm::quat& get_rotation() const { return m_rotation; }

		PE_API const glm::vec3& get_scale() const { return m_scale; }

		PE_API const glm::mat4& matrix() const;

		PE_API operator const glm::mat4& () const;

	protected:
		void update() const;

	protected:
		glm::vec3 m_position{ 0, 0, 0 };
		glm::quat m_rotation = glm::identity<glm::quat>();
		glm::vec3 m_scale{ 1.f, 1.f, 1.f };

		mutable bool m_isDirty{ true };
		mutable glm::mat4 m_matrix = glm::identity<glm::mat4>();

	};

}

