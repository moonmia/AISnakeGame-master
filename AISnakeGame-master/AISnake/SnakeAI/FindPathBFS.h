#pragma once
#include <queue>
using std::queue;

typedef struct
{
	int x;
	int y;
}COORDINATE;

class FindPathBFS
{
private:
	bool **m_chess;//Graph represented by matrix. M_ Chess is a two-dimensional array, 
	//where false indicates path and true indicates no connection
	bool **m_visit;//Test if the current point is visited or not
	COORDINATE **m_parent;//Every visited point's parent point.
	COORDINATE m_size;//The size of map
	void FindPath(COORDINATE begin_point, COORDINATE end_point);
	void GetQueue(COORDINATE end_point);

public:
	FindPathBFS();
	
	~FindPathBFS();

	void InitMap(bool **chess);
	void GetPath(COORDINATE begin_point, COORDINATE end_point);

	queue<COORDINATE> m_paths_queue;

};

