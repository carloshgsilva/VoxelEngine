#pragma once


class DeltaTime {
	float m_Time;
public:
	DeltaTime(float time = 0.0f) : m_Time(time) {

	}

	operator float() const { return m_Time; }

	float GetSeconds() const { return m_Time; }
	float GetMilliseconds() const { return m_Time*1000.0f; }
};