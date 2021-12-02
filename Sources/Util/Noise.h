#pragma once


struct CellsResult {
	float CurrentNoise;
	float CurrentDistance;
	float Distance[4];
	float Noise[4];
};


struct TerrainNoiseInfo {
	float Bias2D;
	float Scale2D;
	float Frequency2D;
	int Octaves2D;

	float Bias3D;
	float Scale3D;
	float Frequency3D;
	int Octaves3D;
};

class Noise {
public:

	static float GetOctave(float x, float y, int octaves);
	static float GetOctave(float x, float y, float z, int octaves);
	static float Get(float x, float y);
	static float Get(float x, float y, float z);
	static float GetTerrainNoise(float x, float y, float z, TerrainNoiseInfo& info);
	static CellsResult GetCells(float x, float y);
};