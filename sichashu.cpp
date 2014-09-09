#include<stdio.h>
#include<iostream>
#include<vector>
#include<string>
#include<cmath>
#include<algorithm>
#include<fstream>
#include<stdlib.h>
#include <direct.h>
using namespace std;
long COUNT=0;
long FLAG1=0;
long FLAG2=0;
const int nodeNum=6105;
const double PI=3.1415926;
//将地图的边界X，Y平均分64等份，最大level为6，最小为0  
const int pice=128;
const double xLeft=281.;
const double yUp=3935.;
const double xRight=23854.;
const double yDown=30851.;
const double Dx=(xRight-xLeft)/(pice*2);
const double Dy=(yDown-yUp)/(pice*2);
class A{//A,B用于最后的结果分析
public:
	double x;
	int y;
	char a[16];
	double z;
	A():x(0.0),y(0),z(0){}
};
class B{
public:
	int k;
	int sucess;
	int failed;
	int num;
	double p;
	double s;
	double avS;
	B():k(0),sucess(0),failed(0),num(0),p(0),s(0),avS(0){}
};
class area{//区域
public:
	double midX;
	double midY;
	double dx;//中心离边界的距离
	double dy;
	int level;//所在层
	int userNumber;//区域中上个时刻有请求的用户数
	area(){
		userNumber=0;
		level=0;
		dx=Dx;
		dy=Dy;
	}
	area getparent(area Area){
		area parentArea;
		if((int)((Area.midX+Area.dx)/(2*Area.dx)+0.5)%2==1&&(int)((Area.midY+Area.dy)/(2*Area.dy)+0.5)%2==1){
			parentArea.midX=Area.midX+Area.dx;
			parentArea.midY=Area.midY+Area.dy;
		}
		if((int)((Area.midX+Area.dx)/(2*Area.dx)+0.5)%2==1&&(int)((Area.midY+Area.dy)/(2*Area.dy)+0.5)%2==0){
			parentArea.midX=Area.midX+Area.dx;
			parentArea.midY=Area.midY-Area.dy;
		}
		if((int)((Area.midX+Area.dx)/(2*Area.dx)+0.5)%2==0&&(int)((Area.midY+Area.dy)/(2*Area.dy)+0.5)%2==1){
			parentArea.midX=Area.midX-Area.dx;
			parentArea.midY=Area.midY+Area.dy;
		}
		if((int)((Area.midX+Area.dx)/(2*Area.dx)+0.5)%2==0&&(int)((Area.midY+Area.dy)/(2*Area.dy)+0.5)%2==0){
			parentArea.midX=Area.midX-Area.dx;
			parentArea.midY=Area.midY-Area.dy;
		}
		parentArea.dx=2*Area.dx;
		parentArea.dy=2*Area.dy;
		parentArea.level=Area.level++;
		return parentArea;
	}
};
class Point{//用户记录
public:
	char lei_x[16];
	int bian_h;
	int ji_lu;
	int shu_x;
	int time;
	double x;
	double y;
	double speed;
	int x1;
	int y1;
	bool isIn;//在区域扩张的时候用
	Point(){
	x=0.;
	y=0.;
	speed=0.;
	isIn=false;
	};
};
class Node{//地图节点
public:
	int bianHao;
	int x;
	int y;
	int num;
	int connectNode[5];//所有相连接的Node的编号
	vector<double> p;
	Node(){
		num=0;
	}
};
class readNode{//读节点
public:
	int number;
	int x;
	int y;
	char ch;
};
class connect{//读节点连接文件
public:
	int y1;
	int y2;
	int y3;
	int y4;
	int y5;
	int y6;
};
class P{//选择路径的起始节点
public:
	Node *newNodeStart;
	Node *newNodeEnd;
	double p;
	double goDis;//已经走的距离
	bool goOn;//是否需要继续计算
	double inDis;//恰好进入小区域的距离		或再次进来的距离
	double outDis;//进去后又出来的距离     或刚好出去的距离
	int crs;//交点个数
	double orignSpeed;
	P(){
		newNodeStart=NULL;
		newNodeEnd=NULL;
		p=1;
		goDis=0;
		goOn=true;
		crs=0;
		inDis=0;
		outDis=0;
		orignSpeed=0;
	}
};
class Store{//存储历史情况
public:
	int bianHao;
	int time;
	int fileSeek;
	//double speed;
	Store(){
		bianHao=0;
		time=0;
		fileSeek=0;
		//speed=0;
	}
};
class startAndEndNode{//起始节点
public:
	Node *nodeStart;
	Node *nodeEnd;
	startAndEndNode(){
		nodeStart=NULL;
		nodeEnd=NULL;
	}
};
class Intersect{//求焦点后的返回类型
public:
	int crossNum;//焦点个数
	double minDis;//起点到焦点距离
	double maxDis;
	bool isCross;//是否有交点
	Intersect(){
		crossNum=0;//初始化
		minDis=0;
		maxDis=0;
		isCross=false;
	}
};
class toWhichNode{//走向哪一个节点，用于路口选择
public:
	Node *toNode;
	int n;
	double p;
	toWhichNode(){
		toNode=NULL;
		n=0;
		p=0;
	}
};

Node Nodes[nodeNum];
area Area[pice][pice];

//计算指定区域，指定时刻 活跃用户数（假设编号为偶数的用户活跃）
int computeUserNumber(area *A,int time,FILE* pfile){
	Point pt;
	//FILE *pfile=fopen("new.dat","rb");
	rewind(pfile);
	char buf[256];
	int n=0;
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
			&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1)	;
		if(pt.time==time&&pt.x>=A->midX-A->dx&&pt.x<=A->midX+A->dx&&
			pt.y>=A->midY-A->dy&&pt.y<=A->midY+A->dy&&pt.bian_h%2==1)//在Area的范围之内,假设编号为偶数的用户活跃
			n++;
		if(pt.time>time)
			break;
	}
//	fclose(pfile);
	return n;
}
//计算指定区域，指定时刻,用户数
int computeAllUserNum(area *A,int time,FILE* pfile){
	Point pt;
	//FILE *pfile=fopen("new.dat","rb");
	rewind(pfile);
	char buf[256];
	rewind(pfile);
	int n=0;
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
			&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1)	;
		if(pt.time==time&&pt.x>=A->midX-A->dx&&pt.x<=A->midX+A->dx&&
			pt.y>=A->midY-A->dy&&pt.y<=A->midY+A->dy)//在Area的范围之内,所有用户
			n++;
		if(pt.time>time)
			break;
	}
	//fclose(pfile);
	return n;
}
//将node文件存储
void nodeInitiation(){
	readNode n;
	char buf[256];
	FILE *pfile=fopen("node.txt","rb");
	rewind(pfile);
	int nodeId=0;
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%d\t%d\t%d\t%c",&n.number,&n.x,&n.y,&n.ch);
		Nodes[nodeId].bianHao=n.number;
		Nodes[nodeId].x=n.x;
		Nodes[nodeId].y=n.y;
		nodeId++;
	}
	fclose(pfile);
}
//将点与点的关系存储
void connectNodeInitia(){
	connect con;
	FILE *pfile=fopen("connection.txt","rb");
	rewind(pfile);
	char buf[256];
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%d\t%d\t%d\t%d\t%d\t%d",&con.y1,&con.y2,&con.y3,&con.y4,&con.y5,&con.y6);
		for(int i=0;i<nodeNum;i++){
			int flag=0;
			if(Nodes[i].bianHao==con.y1){
				Nodes[i].connectNode[Nodes[i].num]=con.y4;
				Nodes[i].num++;
				flag++;
			}
			if(Nodes[i].bianHao==con.y4){
				Nodes[i].connectNode[Nodes[i].num]=con.y1;
				Nodes[i].num++;
				flag++;
			}
			if(flag==2)
				break;
		}
	}
	fclose(pfile);
}
//点是否在区域内
bool isInArea(Node node,area A){
	if(node.x>=A.midX-A.dx&&node.x<=A.midX+A.dx&&
		node.y>=A.midY-A.dy&&node.y<=A.midY+A.dy)
		return true;
	else
		return false;
}
//底层区域初始化
void areaInitia(){
	for(int i=0;i<pice;i++)
		for(int j=0;j<pice;j++){
			Area[i][j].midX=(2*i+1)*Dx+xLeft;
			Area[i][j].midY=(2*j+1)*Dy+yUp;
		}
}
//确定某一时刻，某个用户在哪个区域
area *whichArea(int time,int userId,FILE* pfile){
	Point pt;
	//FILE *pfile=fopen("new.dat","rb");
	rewind(pfile);
	char buf[256];
	rewind(pfile);
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
			&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
		if(pt.time==time&&pt.bian_h==userId){
			return &Area[(int)(0.5*(pt.x-xLeft)/Dx)][(int)(0.5*(pt.y-yUp)/Dy)];
			break;
		}
	}
	//fclose(pfile);
}
//判断是否要进行预测
bool isPrediction(int time,int userId,int k,area *Ar,FILE* pfile){
	if(computeUserNumber(Ar,time,pfile)>=k)
		return false;
	else
		return true;
}
//预测
//step1：按最大速度将区域扩张，得到上一时刻活跃但当前时刻不活跃的用户列表 Point类型
vector<Point>vPoint;//用容器vPoint存储大区域内小区域外上一时刻活跃但当前时刻不活跃的用户
bool pointIsIn(Point pt,double x,double y,double X,double Y){//判断用户是否在一个指定的区域内
	if(pt.x>x&&pt.y>y&&pt.x<X&&pt.y<Y)
		return true;
	else
		return false;
}
void expand(area *Are,int time,double maxSp,FILE* pfile){//扩大区域，初始化vPoint
	double x,X;
	double y,Y;
	x=Are->midX-Are->dx-maxSp;
	X=Are->midX+Are->dx+maxSp;
	y=Are->midY-Are->dy-maxSp;
	Y=Are->midY+Are->dy+maxSp;
	//FILE *pfile=fopen("new.dat","rb");
	rewind(pfile);
	char buf[256];
	rewind(pfile);
	Point pt;
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
			&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
		if(pt.time==time-1){
			if(pt.lei_x[0]=='p'&&pointIsIn(pt,x,y,X,Y)&&!pointIsIn(pt,x+maxSp,y+maxSp,X-maxSp,Y-maxSp)&&pt.bian_h%2==0)//假设编号为奇数的用户当前时刻不活跃
				vPoint.push_back(pt);
			if(pt.lei_x[0]=='p'&&pointIsIn(pt,x+maxSp,y+maxSp,X-maxSp,Y-maxSp)&&pt.bian_h%2==0){
				pt.isIn=true;
				vPoint.push_back(pt);
			}
		}
		if(pt.time==time)
			break;
	}
	//fclose(pfile);
}

//通过node的编号找到node[]
Node* idToNode(int id){
	for(int i=0;i<nodeNum;i++){
		if(Nodes[i].bianHao==id){
			return &Nodes[i];
		}
	}
}
//通过node的坐标找到node[]
Node* pointToNode(int x,int y){
	for(int i=0;i<nodeNum;i++){
		if(Nodes[i].x==x&&Nodes[i].y==y){
			return &Nodes[i];
		}
	}
}
//通过起始*nodeStart和*nodeEnd找出所有出路
vector<toWhichNode> computeToWhichNode(Node *nodeStart,Node *nodeEnd){
	vector<toWhichNode>vToWhichNode;
	if(nodeEnd->num==1){
		cout<<"no way!";
		return vToWhichNode;
	}
	else{
		toWhichNode twn;
		for(int i=0;i<nodeEnd->num;i++){
			if(nodeEnd->connectNode[i]!=nodeStart->bianHao){
				twn.toNode=idToNode(nodeEnd->connectNode[i]);
				vToWhichNode.push_back(twn);
			}
		}
		return vToWhichNode;
	}
}

//根据userId和time计算nodeStart和nodeEnd
startAndEndNode showStartAndEnd(int time,int userId,FILE* pfile){
	startAndEndNode startAndEnd;
	Point pt;
	long fileseek=0;
	//FILE *pfile=fopen("new.dat","rb");
	rewind(pfile);
	char buf[256];
	rewind(pfile);
	while(fgets(buf,256,pfile)){
		fileseek++;
		sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
			&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
		if(pt.time==time&&pt.bian_h==userId){
			startAndEnd.nodeEnd=pointToNode(pt.x1,pt.y1);
			break;
		}
	}
	//fclose(pfile);
	for(int j=0;j<startAndEnd.nodeEnd->num;j++){
		Node *pN=idToNode(startAndEnd.nodeEnd->connectNode[j]);
		cout<<"Find start and end node:"<<(int)((pt.x-pt.x1)*(pt.y-pN->y))<<" "<<(int)((pt.x-pN->x)*(pt.y-pt.y1))<<endl;
		if((int)((pt.x-pN->x)*(pt.y-pt.y1))==(int)((pt.x-pt.x1)*(pt.y-pN->y))&&(pt.x-pt.x1)*(pt.x-pN->x)<=0&&
			(pt.y-pt.y1)*(pt.y-pN->y)<=0){
			startAndEnd.nodeStart=pN;
			break;
		}
		/*Node *pN=idToNode(startAndEnd.nodeEnd->connectNode[j]);
		cout<<"nodeEnd和第"<<j<<"个连接节点的斜率为："<<(double)(startAndEnd.nodeEnd->y-pN->y)/(double)(startAndEnd.nodeEnd->x-pN->x)<<" "<<pN->bianHao
			<<" "<<pN->x<<" "<<pN->y<<endl;*/

	}
	if(startAndEnd.nodeStart==NULL||startAndEnd.nodeEnd==NULL){
		cout<<"error when computing startAndEnd"<<endl;
		system("pause");
	}
	return startAndEnd;
}
//void probInitia(FILE* pfile,int time_start,int time_end){
//	ofstream outfile("proOut.dat");
//	rewind(pfile);
//	for(int i=0;i<nodeNum;++i){
//		for(int j=0;j<Nodes[i].num;++j){
//			Node *endNode=idToNode(Nodes[i].connectNode[j]);
//			vector<toWhichNode> vToWhichNode=computeToWhichNode(&Nodes[i],endNode);
//			if(vToWhichNode.empty())
//				outfile<<2<<'\t'<<Nodes[i].bianHao<<'\t'<<endNode->bianHao<<'\t'<<0<<'\n';//第一个为2表示endNode为结束节点
//			else if(vToWhichNode.size()==1)//不是路口
//				outfile<<1<<'\t'<<Nodes[i].bianHao<<'\t'<<endNode->bianHao<<'\t'<<vToWhichNode[0].toNode->bianHao<<'\n';
//			else{
//				vector<Store> vStore;
//				Store s;
//				char buf[256];
//				Point pt;
//				fseek(pfile,0,SEEK_SET);
//				while(fgets(buf,256,pfile)){
//					sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
//						&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
//					if(pt.time>=time_start&&pt.time<=time_end){
//						if(endNode->x==pt.x1&&endNode->y==pt.y1&&(int)((pt.x-Nodes[i].x)*(pt.y-endNode->y))==
//							(int)((pt.x-endNode->x)*(pt.y-Nodes[i].y))&&(pt.x-Nodes[i].x)*(pt.x-endNode->x)<0){
//								s.bianHao=pt.bian_h;
//								s.fileSeek=ftell(pfile);
//								s.time=pt.time;
//								vStore.push_back(s);			
//						}
//					}
//				}
//				int total=0;
//				for(vector<Store>::iterator i=vStore.begin();i!=vStore.end();++i){
//					fseek(pfile,i->fileSeek,SEEK_SET);
//					while(fgets(buf,256,pfile)){
//						sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
//						&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
//						if(pt.bian_h==i->bianHao&&(pt.x1!=endNode->x||pt.y1!=endNode->y)){
//							for(vector<toWhichNode>::iterator i=vToWhichNode.begin();i!=vToWhichNode.end();i++){
//								if(i->toNode->x==pt.x1&&i->toNode->y==pt.y1){
//									i->n++;
//									break;
//								}
//							}
//							break;
//						}
//					}
//				}
//				//计算概率
//				for(vector<toWhichNode>::iterator i=vToWhichNode.begin();i!=vToWhichNode.end();i++)
//					total+=i->n;
//				for(vector<toWhichNode>::iterator pV=vToWhichNode.begin();pV!=vToWhichNode.end();++pV){
//					if(total==0)
//						outfile<<pV->n/1<<'\t'<<Nodes[i].bianHao<<'\t'<<endNode->bianHao<<'\t'<<pV->toNode->bianHao<<'\n';
//					else
//						outfile<<(double)pV->n/total<<'\t'<<Nodes[i].bianHao<<'\t'<<endNode->bianHao<<'\t'<<pV->toNode->bianHao<<'\n';
//				}
//				
//			}
//		}
//	}
//	outfile.close();
//}

//根据历史信息计算在路口向各个方向运动的概率
/*
vector<P> computePr(int time ,vector<P>::iterator vp){//sGone为已经经过的距离
	double computeDistance(double x,double y,double x1,double y1);
	vector<P> prob;
	vector<Store>vStore;//存储历史情况
	vector<toWhichNode> vToWhichNode=computeToWhichNode(vp->newNodeStart,vp->newNodeEnd);
	if(vToWhichNode.empty()){//已经到了无法分裂
		prob.push_back(*vp);
		prob[0].goOn=false;
	}
	else{
		if(vToWhichNode.size()==1){
			P pr;
			pr.newNodeStart=vp->newNodeEnd;
			pr.newNodeEnd=vToWhichNode.front().toNode;
			pr.p=vp->p;
			pr.goOn=vp->goOn;
			pr.crs=vp->crs;
			pr.inDis=vp->inDis;
			pr.outDis=vp->outDis;
			pr.orignSpeed=vp->orignSpeed;
			pr.goDis=computeDistance((double)pr.newNodeStart->x,(double)pr.newNodeStart->y,(double)pr.newNodeEnd->x,(double)pr.newNodeEnd->y)
				+vp->goDis;
			prob.push_back(pr);
		}
		else{
			Point pt;
			Store s;
			FILE *pfile=fopen("new.dat","rb");
			char buf[256];
			while(fgets(buf,256,pfile)){
				sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
					&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
				if(pt.time<time&&vp->newNodeEnd->x==pt.x1&&vp->newNodeEnd->y==pt.y1&&(int)((pt.x-vp->newNodeStart->x)*(pt.y-vp->newNodeEnd->y))==
					(int)((pt.x-vp->newNodeEnd->x)*(pt.y-vp->newNodeStart->y))&&(pt.x-vp->newNodeStart->x)*(pt.x-vp->newNodeEnd->x)<0){//统计time之前的历史情况
					//如果从nodeStart向nodeEnd运动 锁定
					s.fileSeek=ftell(pfile);
					s.bianHao=pt.bian_h;
					s.time=pt.time ;
					vStore.push_back(s);
				}
			}
			int total=0;
			for(vector<Store>::iterator i=vStore.begin();i!=vStore.end();++i){
				fseek(pfile,(*i).fileSeek,SEEK_SET);
				while(fgets(buf,256,pfile)){
					sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
					&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
					if(pt.time<time&&pt.bian_h==i->bianHao&&(pt.x1!=vp->newNodeEnd->x||pt.y1!=vp->newNodeEnd->y)){
						//total++;
						for(vector<toWhichNode>::iterator i=vToWhichNode.begin();i!=vToWhichNode.end();i++){
							if((*i).toNode->x==pt.x1&&(*i).toNode->y==pt.y1){
								(*i).n++;
								break;
							}
						}
						break;
					}
				}
			}
				//计算概率
			for(vector<toWhichNode>::iterator i=vToWhichNode.begin();i!=vToWhichNode.end();i++){
				total+=i->n;
			}
			if(total==0){
				cout<<"分裂失败->";
				prob.push_back(*vp);
				prob[0].goOn=false;
			}
			else{
				for(vector<toWhichNode>::iterator i=vToWhichNode.begin();i!=vToWhichNode.end();i++){
					P pr;
					pr.newNodeStart=vp->newNodeEnd;
					pr.newNodeEnd=(*i).toNode;
					pr.p=vp->p*(double)i->n/total;
					pr.goOn=vp->goOn;
					pr.inDis=vp->inDis;
					pr.orignSpeed=vp->orignSpeed;
					pr.outDis=vp->outDis;
					pr.crs=vp->crs;
					pr.goDis=computeDistance((double)pr.newNodeStart->x,(double)pr.newNodeStart->y,(double)pr.newNodeEnd->x,(double)pr.newNodeEnd->y)
						+vp->goDis;
					prob.push_back(pr);
				}
			}
		}
	}
	return prob;
}*/
class pOut{
public:
	double p;
	int nSt;
	int nEd;
	int nTo;
	pOut(){
		p=0.;
		nSt=0;
		nEd=0;
		nTo=0;
	}
};
vector<P> computePr(int time ,vector<P>::iterator vp,FILE *pfile){//sGone为已经经过的距离
	double computeDistance(double x,double y,double x1,double y1);
	rewind(pfile);
	char buf[256];
	vector<P> prob;
	pOut po;
	vector<toWhichNode> vToWhichNode;
	toWhichNode s;
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%lf\t%d\t%d\t%d\t",&po.p,&po.nSt,&po.nEd,&po.nTo);
		if(po.nSt==vp->newNodeStart->bianHao&&po.nEd==vp->newNodeEnd->bianHao&&po.p!=2){
			s.p=po.p;
			s.toNode=idToNode(po.nTo);
			vToWhichNode.push_back(s);
		}
	}
	//fclose(pfile);
	if(vToWhichNode.empty()){//已经到了无法分裂
		prob.push_back(*vp);
		prob[0].goOn=false;
	}
	else{
		for(vector<toWhichNode>::iterator i=vToWhichNode.begin();i!=vToWhichNode.end();++i){
			P pr;
			pr.newNodeStart=vp->newNodeEnd;
			pr.newNodeEnd=i->toNode;
			pr.p=vp->p*i->p;
			pr.goOn=vp->goOn;
			pr.crs=vp->crs;
			pr.inDis=vp->inDis;
			pr.outDis=vp->outDis;
			pr.orignSpeed=vp->orignSpeed;
			pr.goDis=computeDistance((double)pr.newNodeStart->x,(double)pr.newNodeStart->y,(double)pr.newNodeEnd->x,(double)pr.newNodeEnd->y)
				+vp->goDis;
			prob.push_back(pr);
		}
	}
	return prob;
}
vector<double> vprob;
bool isNodeIn(Node *node,double x,double y,double X,double Y){
	if(node->x>x&&node->y>y&&node->x<X&&node->y<Y)
		return true;
	else
		return false;
}
Intersect computeIntersect(Node *nodeStart,Node *nodeEnd,area *Ar){
	Intersect reIntersect;
	double k,b,d;
	if(nodeEnd->y-nodeStart->y==0)//平行于X轴
	{
		if(nodeEnd->y==Ar->midY+Ar->dy || nodeEnd->y==Ar->midY-Ar->dy)//和上下边框重合
		{
			if(nodeEnd->x>=Ar->midX-Ar->dx && nodeEnd->x<=Ar->midX+Ar->dx)//终点在边框上
			{
				if(nodeStart->x<Ar->midX-Ar->dx)
				{
					reIntersect.crossNum=2;
					reIntersect.isCross=true;
					reIntersect.minDis=0;
					reIntersect.maxDis=nodeEnd->x-(Ar->midX-Ar->dx);
				}
				if(nodeStart->x>Ar->midX+Ar->dx)
				{
					reIntersect.crossNum=2;
					reIntersect.isCross=true;
					reIntersect.minDis=0;
					reIntersect.maxDis=Ar->midX+Ar->dx-nodeEnd->x;
				}
			}
			if(nodeStart->x>=Ar->midX-Ar->dx && nodeStart->x<=Ar->midX+Ar->dx)//起点在边框上
			{
				if(nodeEnd->x<Ar->midX-Ar->dx)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=Ar->midX-Ar->dx-nodeEnd->x;
				}
				if(nodeEnd->x>Ar->midX+Ar->dx)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=nodeEnd->x-(Ar->midX+Ar->dx);
				}
			}
		}
		if(nodeEnd->y>Ar->midY-Ar->dy && nodeEnd->y<Ar->midY+Ar->dy)
		{
			if(nodeStart->x>=Ar->midX-Ar->dx && nodeStart->x<=Ar->midX+Ar->dx)
			{
				if(nodeEnd->x<Ar->midX-Ar->dx)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=Ar->midX-Ar->dx-nodeEnd->x;
				}
				if(nodeEnd->x>Ar->midX+Ar->dx)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=nodeEnd->x-(Ar->midX+Ar->dx);
				}
			}
			if(nodeEnd->x>=Ar->midX-Ar->dx && nodeEnd->x<=Ar->midX+Ar->dx)
			{
				if(nodeStart->x<Ar->midX-Ar->dx)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=nodeEnd->x-(Ar->midX-Ar->dx);
				}
				if(nodeStart->x>Ar->midX+Ar->dx)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=Ar->midX+Ar->dx-nodeEnd->x;
				}
			}
			if(nodeStart->x<Ar->dx-Ar->midX&&nodeEnd->x>Ar->midX-Ar->dx){
					reIntersect.crossNum=2;
					reIntersect.isCross=true;
					reIntersect.maxDis=nodeEnd->x-(Ar->midX-Ar->dx);
					reIntersect.minDis=nodeEnd->x-(Ar->midX+Ar->dx);
			}
			if(nodeEnd->x<Ar->dx-Ar->midX&&nodeStart->x>Ar->midX-Ar->dx){
					reIntersect.crossNum=2;
					reIntersect.isCross=true;
					reIntersect.maxDis=(Ar->midX+Ar->dx)-nodeEnd->x;
					reIntersect.minDis=(Ar->midX-Ar->dx)-nodeEnd->x;
			}
		}
	}
	else if(nodeEnd->x-nodeStart->x==0)//平行于y轴
	{
		if(nodeEnd->x==Ar->midX+Ar->dx || nodeEnd->x==Ar->midX-Ar->dx)//与边框重合
		{
			if(nodeEnd->y>=Ar->midY-Ar->dy && nodeEnd->y<=Ar->midY+Ar->dy)
			{
				if(nodeStart->y<Ar->midY-Ar->dy)
				{
					reIntersect.crossNum=2;
					reIntersect.isCross=true;
					reIntersect.minDis=0;
					reIntersect.maxDis=nodeEnd->y-(Ar->midY-Ar->dy);
				}
				if(nodeStart->y>Ar->midY+Ar->dy)
				{
					reIntersect.crossNum=2;
					reIntersect.isCross=true;
					reIntersect.minDis=0;
					reIntersect.maxDis=Ar->midY+Ar->dy-nodeEnd->y;
				}
			}
			if(nodeStart->y>=Ar->midY-Ar->dy && nodeStart->y<=Ar->midY+Ar->dy)
			{
				if(nodeEnd->y<Ar->midY-Ar->dy)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=Ar->midY-Ar->dy-nodeEnd->y;
				}
				if(nodeEnd->y>Ar->midY+Ar->dy)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=nodeEnd->y-(Ar->midY+Ar->dy);
				}
			}
		}
		if(nodeEnd->x>Ar->midX-Ar->dx && nodeEnd->x<Ar->midX+Ar->dx)
		{
			if(nodeStart->y>=Ar->midY-Ar->dy && nodeStart->y<=Ar->midY+Ar->dy)
			{
				if(nodeEnd->y<Ar->midY-Ar->dy)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=Ar->midY-Ar->dy-nodeEnd->y;
				}
				if(nodeEnd->y>Ar->midY+Ar->dy)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=nodeEnd->y-(Ar->midY+Ar->dy);
				}
			}
			if(nodeEnd->y>=Ar->midY-Ar->dy && nodeEnd->y<=Ar->midY+Ar->dy)
			{
				if(nodeStart->y<Ar->midY-Ar->dy)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=nodeEnd->y-(Ar->midY-Ar->dy);
				}
				if(nodeStart->y>Ar->midY+Ar->dy)
				{
					reIntersect.crossNum=1;
					reIntersect.isCross=true;
					reIntersect.maxDis=Ar->midY+Ar->dy-nodeEnd->y;
				}
			}
		}
	}
	else//普通情况
	{
		k=((double)(nodeEnd->y-nodeStart->y))/((double)(nodeEnd->x-nodeStart->x));
		b=nodeStart->y-k*nodeStart->x;

		double result_x,result_y;

		result_x=Ar->midX-Ar->dx;//左边框交点
		result_y=k*result_x+b;
		if((result_x>=nodeStart->x && result_x<=nodeEnd->x) || (result_x<=nodeStart->x &&result_x>=nodeEnd->x))//交点在线段上
		{
			if(result_y>=Ar->midY-Ar->dy  && result_y<=Ar->midY+Ar->dy)//且交点在左边框
			{
				reIntersect.crossNum++;
				reIntersect.isCross=true;
				reIntersect.maxDis=sqrt((result_x-nodeEnd->x)*(result_x-nodeEnd->x)
												+(result_y-nodeEnd->y)*(result_y-nodeEnd->y));
			}
		}

		result_x=Ar->midX+Ar->dx;//右边框交点
		result_y=k*result_x+b;
		if((result_x>=nodeStart->x && result_x<=nodeEnd->x) || (result_x<=nodeStart->x &&result_x>=nodeEnd->x))
		{
			if(result_y>=Ar->midY-Ar->dy && result_y<=Ar->midY+Ar->dy)
			{
				reIntersect.crossNum++;
				d=sqrt((result_x-nodeEnd->x)*(result_x-nodeEnd->x)
							   +(result_y-nodeEnd->y)*(result_y-nodeEnd->y));
				if(reIntersect.isCross)
				{
					if(reIntersect.maxDis<d)
					{
						reIntersect.minDis=reIntersect.maxDis;
						reIntersect.maxDis=d;
					}
					else
					{
						reIntersect.minDis=d;
					}
				}
				else
				{
					reIntersect.isCross=true;
					reIntersect.maxDis=d;
				}
			}
		}

		result_y=Ar->midY+Ar->dy;
		result_x=(result_y-b)/k;
		if((result_y>=nodeStart->y && result_y<=nodeEnd->y) || (result_y<=nodeStart->y &&result_y>=nodeEnd->y))
		{
			if(result_x>Ar->midX-Ar->dx  && result_x<Ar->midX+Ar->dx)
			{
				reIntersect.crossNum++;
				d=sqrt((result_x-nodeEnd->x)*(result_x-nodeEnd->x)
							   +(result_y-nodeEnd->y)*(result_y-nodeEnd->y));
				if(reIntersect.isCross)
				{
					if(reIntersect.maxDis<d)
					{
						reIntersect.minDis=reIntersect.maxDis;
						reIntersect.maxDis=d;
					}
					else
					{
						reIntersect.minDis=d;
					}
				}
				else
				{
					reIntersect.isCross=true;
					reIntersect.maxDis=d;
				}
			}
		}

		result_y=Ar->midY-Ar->dy;
		result_x=(result_y-b)/k;
		if((result_y>=nodeStart->y && result_y<=nodeEnd->y) || (result_y<=nodeStart->y &&result_y>=nodeEnd->y))
		{
			if(result_x>Ar->midX-Ar->dx && result_x<Ar->midX+Ar->dx)
			{
				reIntersect.crossNum++;
				d=sqrt((result_x-nodeEnd->x)*(result_x-nodeEnd->x)
							   +(result_y-nodeEnd->y)*(result_y-nodeEnd->y));
				if(reIntersect.isCross)
				{
					if(reIntersect.maxDis<d)
					{
						reIntersect.minDis=reIntersect.maxDis;
						reIntersect.maxDis=d;
					}
					else
					{
						reIntersect.minDis=d;
					}
				}
				else
				{
					reIntersect.isCross=true;
					reIntersect.maxDis=d;
				}
			}
		}
		double d_two;
		d_two=sqrt((double)((nodeStart->x-nodeEnd->x)*(nodeStart->x-nodeEnd->x)+
			                (nodeStart->y-nodeEnd->y)*(nodeStart->y-nodeEnd->y)));
		if(reIntersect.crossNum==2 && reIntersect.maxDis==d_two)
		{
			reIntersect.crossNum=1;
			reIntersect.maxDis=reIntersect.minDis;
			reIntersect.minDis=0;
		}

	}
	//中间为计算过程
	cout<<reIntersect.crossNum<<" "<<reIntersect.maxDis<<" "<<reIntersect.minDis<<endl;
	return reIntersect;//最后返回这个结果
}
//标准正态分布
double Normal(double z){
	return exp((-1)*z*z/2)/sqrt(2*PI);
}
double NormSDist(const double z){
	if(z > 6) return 1;
	if(z < -6) return 0; 
	static const double gamma =  0.231641900,
		a1  =  0.319381530,
		a2  = -0.356563782,
		a3  =  1.781477973,
		a4  = -1.821255978,
		a5  =  1.330274429; 
	double k = 1.0 / (1 + fabs(z) * gamma);
	double n = k * (a1 + k * (a2 + k * (a3 + k * (a4 + k * a5))));
	n = 1 - Normal(z) * n;
	if(z < 0)
		return 1.0 - n; 
	return n;
}
//通过范围求概率
double disToPr(double s1,double s2,double orignSp/*,double u,double fangC*/){//参数为速度范围，原始速度，最大速度,均值和方差
	return fabs(NormSDist((s2-orignSp)/100)-NormSDist((s1-orignSp)/100));
}
vector<double> computeSpeedRange(vector<Point> *pvPoint,int time,double maxSpe,area *Ar,FILE* prf,FILE* pfile){
	double computeDistance(double x,double y,double x1,double y1);
	vector<double> probabilty;
	for(vector<Point>::iterator i=(*pvPoint).begin();i!=(*pvPoint).end();++i){
		startAndEndNode pNode=showStartAndEnd((*i).time,(*i).bian_h,pfile);//初始的起始node
		vector<P> vP;
		P orignP;
		orignP.newNodeStart=pNode.nodeStart;
		orignP.newNodeEnd=pNode.nodeEnd;
		orignP.orignSpeed=i->speed;
		orignP.goDis=computeDistance((*i).x,(*i).y,pNode.nodeEnd->x,pNode.nodeEnd->y);
		vP.push_back(orignP);
		bool isGoOn=true;
		//vector<P>::iterator pBegin=vP.begin();
		vector< vector<P> > vv;
		vector<P> vtemp;
		while(isGoOn){
			for(vector<P>::iterator pOfVp=vP.begin();pOfVp!=vP.end();++pOfVp){
				if(pOfVp->goOn){
					Intersect intersect;
					intersect=computeIntersect(pOfVp->newNodeStart,pOfVp->newNodeEnd,Ar);//计算相交情况
					if(intersect.isCross){//有交点
						if(intersect.crossNum==1)
							FLAG1++;
						else
							FLAG2++;
						if(pOfVp->crs==0){
							if(intersect.crossNum==1)
								pOfVp->inDis=pOfVp->goDis-intersect.maxDis;
							else{
								pOfVp->inDis=pOfVp->goDis-intersect.maxDis;
								pOfVp->outDis=pOfVp->goDis-intersect.minDis;
							}
						}
						else//(*pOfVp).crs==1
							pOfVp->outDis=pOfVp->goDis-intersect.maxDis;
					}
					(*pOfVp).crs+=intersect.crossNum;
					if(pOfVp->crs==1){
						if(pOfVp->goDis>=maxSpe)
							pOfVp->outDis=maxSpe;
					}
					if((*pOfVp).crs>=2||(*pOfVp).goDis>=maxSpe||pOfVp->p==0)//如果有了两个交点，或则超过了极限距离，或者不可能走向，停止分裂
						(*pOfVp).goOn=false;
					if(pOfVp->goOn){
						vector<P> vPTemp=computePr(time-1,pOfVp,prf);//一个方向分裂成多个
						cout<<"COUNT:"<<++COUNT<<endl;
						vv.push_back(vPTemp);
					}
					else
						vtemp.push_back(*pOfVp);
				}
				else
					vtemp.push_back(*pOfVp);
			}
			vP.clear();
			for(vector< vector<P> >::iterator pvv=vv.begin();pvv!=vv.end();++pvv){
				for(vector<P>::iterator pv=pvv->begin();pv!=pvv->end();++pv)
					vP.push_back(*pv);
			}
			for(vector<P>::iterator temp=vtemp.begin();temp!=vtemp.end();++temp)
				vP.push_back(*temp);
			vv.clear();
			vtemp.clear();
			vector<P>::size_type count=0;//判断是否不需要再计算
			for(vector<P>::iterator q=vP.begin();q!=vP.end();++q){
				cout<<q->goDis;
				if((*q).goOn==false){
					cout<<" "<<"stop"<<endl;
					++count;
				}
				else
					cout<<'\n';
			}
			if(count==vP.size())
				isGoOn=false;
		}
		cout<<"Choices of user "<<i->bian_h<<endl;
		for(vector<P>::iterator q=vP.begin();q!=vP.end();++q)
			cout<<q->p<<endl;
		//计算概率
		if((*i).isIn){//如果是从里面出去
			double p0=0;
			for(vector<P>::iterator q=vP.begin();q!=vP.end();++q){
				if(q->crs>=2){//出去，有两个交点
					if (!(q->orignSpeed>q->inDis&&q->orignSpeed<q->outDis))
						p0+=q->p;	
				}	
				if(q->crs==1){//出去，有一个交点
					if(q->orignSpeed<q->inDis)
						p0+=q->p;
				}
				if(q->crs==0)//出去无交点
					p0+=q->p;
			}
			if(p0!=0)
				probabilty.push_back(p0);
		}
		else{//进来
			double p0=0;
			for(vector<P>::iterator q=vP.begin();q!=vP.end();++q){
				if(q->crs>=2){
					if (q->orignSpeed>=q->inDis&&q->orignSpeed<=q->outDis){
						p0+=q->p;
					}
				}
				if(q->crs==1){
					if(q->orignSpeed>q->inDis)
						p0+=q->p;
				}
			}
			if(p0!=0)
				probabilty.push_back(p0);
		}
	}
	pvPoint->clear();
 	return probabilty;
}

double computeDistance(double x,double y,double x1,double y1){
	return sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
}
//计算最大速度
double maxSp(FILE* pfile){
	Point pt;
	char buf[256];
	double maxSp=0.;
	//FILE *pfile=fopen("new.dat","rb");
	rewind(pfile);
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
			&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
		if(pt.time>=1&&pt.speed>maxSp)
			maxSp=pt.speed;
	}
	//fclose(pfile);
	return maxSp;
}
//指定k，求有k个用户留在小区域里面的概率
unsigned int f(int n){//求阶乘
	if(n==0)
		return 1;
	unsigned int temp=1;
	for(unsigned int i=n;i!=1;--i)
		temp*=i;
	return temp;
}
double computeResult(int k,vector<double> vprobability){
	double p=0;//容器里面去大于等于k个的概率
	int m=0;//m为1的个数
	vector<double> temp;
	for(vector<double>::iterator i=vprobability.begin();i!=vprobability.end();++i){
		if(*i>=0.99)
			m++;
		else{
			if(*i>0.1)
				temp.push_back(*i);
		}
	}
	if(m>=k)
		p=1;
	else{
		double dq=0.2;
		while(temp.size()>12){
			cout<<"Danger!"<<endl;
			cout<<"You may wait for a few minutes!"<<endl;
			vector<double> t;
			for(vector<double>::iterator pv=temp.begin();pv!=temp.end();++pv){
				if(*pv>dq)
					t.push_back(*pv);
			}
			temp=t;
			dq+=0.1;
		}
		int n=temp.size();
		double *a=new double[n];
		for(int i=0;i<n;++i)
			*(a+i)=temp[i];
		int *b=new int[n];
		for(int i=0;i<n;++i)
			*(b+i)=i;
		for(int i=k-m;i<=n;++i){
			sort(b,b+n);
			long double total=0;
			do{
				long double tem=1.0;
				for(int j=0;j<i;++j)
					tem*=*(a+*(b+j));
				for(int j=i;j<n;++j)
					tem*=1-*(a+*(b+j));
				total+=tem;
			}while(next_permutation(b,b+n));
			p+=total/(f(i)*f(n-i));
		}
		delete[] a;
		delete[] b;
}
	return p;
}
/*void fun(){
	ofstream outfile("test.dat");
	Point pt;
	char buf[256];
	FILE *pfile=fopen("new.dat","rb");
	int a[501]={0};
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
			&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
		a[pt.time]++;
	}
	for(int i=0;i<501;++i)
		outfile<<i<<'\t'<<a[i]<<'\t'<<(double)a[i]/(32*32)<<'\n';
}*/
vector<int> userInitial(FILE* pfile,int t,int start_user,int end_user){
	vector<int> user;
	Point pt;
	char buf[256];
	rewind(pfile);
	long file_seek=0;
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
			&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
		if(pt.lei_x[0]=='p'&&pt.time==t&&pt.bian_h==start_user){
			user.push_back(start_user);
			file_seek=ftell(pfile);
			break;
		}
	}
	fseek(pfile,file_seek,SEEK_SET);
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%s\t%d\t%d\t%d\t%d\t%lf\t%lf\t%lf\t%d\t%d",
			&pt.lei_x,&pt.bian_h,&pt.ji_lu,&pt.shu_x,&pt.time,&pt.x,&pt.y,&pt.speed,&pt.x1,&pt.y1);
		if(pt.bian_h==end_user)
			break;
		if(pt.lei_x[0]=='p')
			user.push_back(pt.bian_h);
	}
	return user;
}
void main(){
	system("color 0f");
	mkdir("result");
	FILE* srcFile=fopen("new.dat","rb");
	FILE* proFile=fopen("proOut.dat","rb");
	ofstream outfile("result\\kresult.dat");
	ofstream outfile1("result\\pro.dat");//检测概率
	int time;
	int startUser,endUser;
	int userId, k,k_add;
	double yuzhi;
	double maxSpeed=maxSp(srcFile);//最大速度
	cout<<"Please enter the time:"<<endl;
	cin>>time;
	cout<<"Please enter a start_user and a end_user:"<<endl;
	cin>>startUser>>endUser;
	vector<int> vUser=userInitial(srcFile,time,startUser,endUser);
	int u_num=0;
	for(vector<int>::iterator i=vUser.begin();i!=vUser.end();++i){
		cout<<*i<<'\t';
		u_num++;
		if(u_num==10){
			u_num=0;
			cout<<'\n';
		}
	}
	cout<<'\n';
	cout<<vUser.size()<<" users for prediction:"<<endl;
	cout<<"Please enter a threshold value"<<endl;
	cin>>yuzhi;
	cout<<"please enter a number k=num+";
	cin>>k_add;
	outfile<<"Time："<<time<<'\t'<<"User_number: "<<vUser.size()<<'\t'<<"Area: "<<pice<<"*"<<pice<<'\t'<<"start_user: "<<
		startUser<<'\t'<<"end_user: "<<endUser<<'\t'<<"threshold value: "<<yuzhi<<'\t'<<"k_add: "<<k_add<<'\n';
	system("pause");
	//int time_start,time_end;
	//cout<<"输入起始时刻："<<endl;
	//cin>>time_start>>time_end;
	nodeInitiation();
	connectNodeInitia();
	//probInitia(srcFile,time_start,time_end);
	//system("pause");
	areaInitia();
	cout<<"Initiation complete"<<endl;
	for(vector<int>::iterator i=vUser.begin();i!=vUser.end();++i){
		outfile1<<*i<<" :";
		userId=*i;
		area origAr=*whichArea(time,userId,srcFile);
		int num=computeUserNumber(&origAr,time,srcFile);
		cout<<"The area where the user "<<userId<<" in:"<<origAr.midX<<" "<<origAr.midY<<endl;
		cout<<"The user_number of the area："<<num<<endl;
		//for(int j=1;j<=8;++j){
			bool isPre=true;
			area Ar=origAr;
			k=num+k_add;
			while(isPre){
				expand(&Ar,time,maxSpeed,srcFile);//扩大区域，初始化vPoint
				cout<<"Expand using MaxSpeed  "<<maxSpeed<<" ，the users for prediction follows（"<<vPoint.size()<<"）："<<endl;
				for(vector<Point>::iterator i=vPoint.begin();i!=vPoint.end();++i)
						cout<<(*i).bian_h<<'	'<<(*i).speed<<'	'<<(*i).x<<'	'<<(*i).y<<endl;
				int k1=k-computeUserNumber(&Ar,time,srcFile);//还需要的用户数
				if(vPoint.size()>k1){//有足够多的用户来预测
					int temp=vPoint.size();
					vprob=computeSpeedRange(&vPoint,time,maxSpeed,&Ar,proFile,srcFile);//对每一个vPoint成员，计算下一时刻在小区域里面的概率
					outfile1<<k<<"、"<<k-k1<<"、"<<k1<<"、"<<temp<<'\t';
					for(vector<double>::iterator pp=vprob.begin();pp!=vprob.end();++pp){
						outfile1<<*pp<<"、";
					}
					double result=computeResult(k1,vprob);
					outfile1<<"*"<<result<<"*";
					if(result>yuzhi){
						outfile1<<"Y"<<'\n';
						double s=4*Ar.dx*Ar.dy;
						if(computeAllUserNum(&Ar,time,srcFile)>k){
							cout<<"Prediction succeed!"<<endl;//与实际数据对比
							outfile<<result<<'\t'<<k<<'\t'<<"succeed"<<'\t';
						}
						else{
							cout<<"Prediction failed!"<<endl;
							outfile<<result<<'\t'<<k<<'\t'<<"failure"<<'\t';
						}
						cout<<"Without prediction,the area："<<endl;
						while(!(computeUserNumber(&Ar,time,srcFile)>k)){
							Ar=Ar.getparent(Ar);
						}
						cout<<Ar.midX<<" "<<Ar.midX<<" "<<Ar.dx<<" "<<Ar.dy<<endl;
						outfile<<(4*Ar.dx*Ar.dy-s)/(4*Ar.dx*Ar.dy)<<'\n';//面积减少百分比
						break;
					}
					else{//否则小区按四叉树扩张
						outfile1<<"N"<<'\n';
						vPoint.clear();
						cout<<"The probability "<<result<<"dissatisfy! Getparent area."<<endl;
						Ar=Ar.getparent(Ar);
						isPre=isPrediction(time ,userId,k,&Ar,srcFile);
						if(!isPre){//如果不需要预测
							outfile<<2<<'\t'<<k<<'\t'<<"succeed"<<'\t'<<0<<'\n';
						}
					}
				}
				else{
					cout<<"The number of user is not enough.Expand!"<<endl;
					vPoint.clear();
					Ar=Ar.getparent(Ar);
					isPre=isPrediction(time ,userId,k,&Ar,srcFile);
					if(!isPre){//如果不需要预测
						outfile<<2<<'\t'<<k<<'\t'<<"succeed"<<'\t'<<0<<'\n';
					}
				}
			}
			//}
	}
	outfile1.close();
	outfile.close();
	fclose(proFile);
	fclose(srcFile);
	
	//分析结果
	ofstream kout("result\\kout.dat");
	ofstream out("result\\kout_orign.txt");
	FILE* pfile=fopen("result\\kresult.dat","rb");
	char buf[256];
	A cA;
	vector<B> vB;
	//找最大的k
	int max_k=0;
	fgets(buf,256,pfile);
	//x为概率，y为k值，z为面积减少的百分比，a为success/failure
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%f\t%d\t%s\t%lf",&cA.x,&cA.y,&cA.a,&cA.z);
		if(cA.y>max_k)
			max_k=cA.y;
	}
	for(int i=1;i<=max_k;++i){
		B b;
		b.k=i;
		vB.push_back(b);
	}
	rewind(pfile);
	fgets(buf,256,pfile);//第一行不读
	int count=0;
	int count2=0;//统计不用预测的情况
	while(fgets(buf,256,pfile)){
		sscanf(buf,"%lf\t%d\t%s\t%lf",&cA.x,&cA.y,&cA.a,&cA.z);
		count++;
		if((int)(cA.x+0.5)==2)
			count2++;
		for(vector<B>::iterator i=vB.begin();i!=vB.end();++i){
			if(i->k==cA.y){
				i->num++;
				if(cA.a[0]=='s'){
					i->sucess++;
					i->s+=cA.z;
				}
				else
					i->failed++;
				break;
			}
		}
	}
	out<<"所有用户数： "<<count<<'\t'<<"不预测的用户数： "<<count2<<'\t'<<(double)count2/(double)count<<'\n';
	for(vector<B>::iterator i=vB.begin();i!=vB.end();++i){
		out<<i->k<<'\t'<<i->sucess<<'\t'<<i->failed<<'\n';
		i->p=(double)i->sucess/i->num;
		if(i->sucess!=0){
			i->avS=i->s/i->sucess;
		}
		else
			i->avS=0;
		kout<<i->k<<'\t'<<i->p<<'\t'<<i->avS<<'\n';
	}
	fclose(pfile);
	kout.close();
	out.close();
}
