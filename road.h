
#include<string>
const int nodeNumber=6105;
const int activeUserDistribution=2;//定义活跃用户的比例
const int kRequirement=6;//定义隐私需求2~（6+1）
const int number=100;//定义仿真用户数
//地图元素的定义
class Node{
public:
	Node(int identity=0,double xCoordinate=0,double yCoordinate=0):id(identity),x(xCoordinate),y(yCoordinate){}
	bool operator<(const Node &node)const{
		return id<node.id;
	}
	bool operator==(const Node &node)const{
		return ((int)x==(int)node.x)&&((int)y==(int)node.y);
	}
	int id;
	double x;
	double y;
};
class  RoadSegment{
public:
	RoadSegment():children(std::string("")){}
	bool operator<(const RoadSegment &roadSegment)const{
		return startNode.x*endNode.x<roadSegment.startNode.x*roadSegment.startNode.y;
	}
	Node startNode;
	Node endNode;
	std::string children;
};
class Record{
public:
	bool operator==(const Record &record)const{
		return bianHao==record.bianHao;
	}
	bool operator<(const Record &record)const{
		return bianHao<record.bianHao;
	}
	std::string type;
	int bianHao;
	int jiLu;
	int shuXing;
	int time;
	double xCoordinate;
	double yCoordinate;
	double speed;
	Node toNode;
};
struct Read{
		int row1;
		int row2;
		int row3;
		int row4;
		int row5;
		int row6;
};