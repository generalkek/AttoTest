#pragma once

namespace math {

	/*
	 * Return random unsigned interger in range [0, x)
	*/
	unsigned int Random(unsigned int x);

	/*
	 * Return random unsigned interger in range [min, max)
	*/
	unsigned int Random(unsigned int min, unsigned int max);

	/*
	 * Return random float in range [0.0, 1.0)
	*/
	float Random();

	/*
	 * Return random float in range [0.0, x)
	*/
	float RandomF(float x);

	/*
	 * Return random float in range [min, max)
	*/
	float RandomF(float min, float max);
}