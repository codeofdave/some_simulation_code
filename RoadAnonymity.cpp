#include<iostream>
#include<fstream>
#include<map>
#include<vector>
#include<utility>
#include<ctime>
#include<cmath>
#include<algorithm>
#include"road.h"
int initRoad(std::vector<RoadSegment>& vRoad){//初始化路段
	std::ifstream file;
	file.open("connection.txt");
	Read m_read;
	while(file>>m_read.row1>>m_read.row2>>m_read.row3>>m_read.row4>>m_read.row5>>m_read.row6){
		Node start(m_read.row1,m_read.row2,m_read.row3);
		Node end(m_read.row4,m_read.row5,m_read.row6);
		RoadSegment roadSegment;
		roadSegment.startNode=start;
		roadSegment.endNode=end;
		vRoad.push_back(roadSegment);
	}
	file.clear();
	file.close();
	return 0;
}

int initNode(std::multimap<Node,Node>& mapNode){//初始化mapNode
	std::ifstream file("connection.txt");
	Read m_read;
	while(file>>m_read.row1>>m_read.row2>>m_read.row3>>m_read.row4>>m_read.row5>>m_read.row6){
		Node node1(m_read.row1,m_read.row2,m_read.row3);
		Node node2(m_read.row4,m_read.row5,m_read.row6);
		mapNode.insert(std::make_pair(node1,node2));
		mapNode.insert(std::make_pair(node2,node1));
	}
 	return 0;
}

RoadSegment seekRoadSegment(const Record& record,const std::vector<RoadSegment>& vRoad){//由记录定位到路段
	for(std::vector<RoadSegment>::const_iterator i=vRoad.begin();i!=vRoad.end();++i){
		if((int)((record.xCoordinate-i->startNode.x)*(record.yCoordinate-i->endNode.y))==(int)((record.xCoordinate-i->endNode.x)*(record.yCoordinate-i->startNode.y))&&
			(record.xCoordinate-i->startNode.x)*(record.xCoordinate-i->endNode.x)<=0&&(record.yCoordinate-i->startNode.y)*(record.yCoordinate-i->endNode.y)<=0)
			return *i;	
	}
}

int ComputerActiveRecords(//指定集合中，在指定路段上的用户数
	std::vector<Record>& activeRecordsOnSegment,
	const std::vector<Record>& activeRecords,
	const RoadSegment& roadSegment
	)
{
	 for(std::vector<Record>::const_iterator i=activeRecords.begin();i!=activeRecords.end();++i){	
		if((int)((i->xCoordinate-roadSegment.startNode.x)*(i->yCoordinate-roadSegment.endNode.y))==(int)((i->xCoordinate-roadSegment.endNode.x)*(i->yCoordinate-roadSegment.startNode.y))&&
			(i->xCoordinate-roadSegment.startNode.x)*(i->xCoordinate-roadSegment.endNode.x)<=0&&(i->yCoordinate-roadSegment.startNode.y)*(i->yCoordinate-roadSegment.endNode.y)<=0)
			activeRecordsOnSegment.push_back(*i);
	 }
	return 0;
}

int ComputerActiveRecords(
	std::vector<Record> &vRecordCurrent,
	std::vector<Record> &vRecordPrevious,
	int time
	)
{//得到活跃用户集
	std::ifstream file("new.dat");
	if(file==NULL)
		std::cout<<"erro when open new.dat!";
	Record record;
	srand(std::time(NULL));
	while(file>>record.type>>record.bianHao>>record.jiLu>>record.shuXing>>record.time>>record.xCoordinate
		>>record.yCoordinate>>record.speed>>record.toNode.x>>record.toNode.y){
			if(record.time==time-1&&std::rand()%activeUserDistribution==0&&record.type.length()==5)
				vRecordPrevious.push_back(record);
			if(record.time==time&&std::rand()%activeUserDistribution==0&&record.type.length()==5)
				vRecordCurrent.push_back(record);
			if(record.time>time)
				break;
	}
	file.close();
	return 0;
}

 //得到上个时刻活跃，当前时刻不活跃的记录，作为为预测用户，并得到预测用户在当前时刻的位置
void GetPredictionRecords(
	 const std::vector<Record>& activeRecordsPrevious,
	 const std::vector<Record>& activeRecordsCurrent,
	 std::vector<Record>& predictionRecordsPrevious,//预测用户
	 std::vector<Record>& predictionRecordsCurrent//校验用户
	 )
{
	 std::vector<Record> temp;
	 for( std::vector<Record>::const_iterator i=activeRecordsPrevious.begin();i!=activeRecordsPrevious.end();++i){
		if(std::find(activeRecordsCurrent.begin(),activeRecordsCurrent.end(),*i)==activeRecordsCurrent.end())
			temp.push_back(*i);
	 }
	std::ifstream file("new.dat");
	if(file==NULL)
		std::cout<<"erro when open new.dat!";
	Record record;
	int currentTime=activeRecordsCurrent[0].time;
	std::vector<Record>::const_iterator result;
	while(file>>record.type>>record.bianHao>>record.jiLu>>record.shuXing>>record.time>>record.xCoordinate
		>>record.yCoordinate>>record.speed>>record.toNode.x>>record.toNode.y){
			if(record.time==currentTime&&record.type.length()==5){
				result=std::find(temp.begin(),temp.end(),record);
				if(result!=temp.end()){
					predictionRecordsPrevious.push_back(*result);
					predictionRecordsCurrent.push_back(record);			
				}
			}
			if(record.time>currentTime)
				break;
	}
}

void InitRequestCurrent(int number,
	std::map<Record,int>& requestCurrent,
	const std::vector<Record>& activeRecordsCurrent
	)
{
	srand(std::time(NULL));
	if(number>activeRecordsCurrent.size())
		std::cout<<"InitrequestCurrent erro!"<<std::endl;
	for(int i=0;i<number;++i)
		requestCurrent[activeRecordsCurrent[i]]=std::rand()%kRequirement+2;
}

void NoPredict(const std::map<Record,int>& requestCurrent,
	const std::vector<RoadSegment>& vRoad,
	const std::multimap<Node,Node>& mapNode,
	const std::vector<Record>& activeRecordsCurrent,
	std::map<std::pair<Record,int>,std::vector<RoadSegment>>& resultWithoutPredict//返回值
	)
{
	for(std::map<Record,int>::const_iterator i=requestCurrent.begin();i!=requestCurrent.end();++i){
		std::vector<RoadSegment> resultSegment;
		RoadSegment firstSegment=seekRoadSegment(i->first,vRoad);//定位当前路段
		resultSegment.push_back(firstSegment);
		std::vector<Record> activeRecordsOnSegment;
		ComputerActiveRecords(activeRecordsOnSegment,activeRecordsCurrent,resultSegment[0]);
		int activeUsers=activeRecordsOnSegment.size();	
		if(activeUsers>i->second){//如果当前路段已经满足需求
			RoadSegment temp=firstSegment;
			std::string str="";
			int breakFlag=0;
			while(true){
				++breakFlag;
				if(breakFlag>10){
					//str="";
					//resultSegment.erase(resultSegment.begin());
					break;
				}
				int num=0;
				if(temp.startNode.x!=temp.endNode.x){
					if(abs(i->first.xCoordinate-temp.startNode.x)>abs(i->first.xCoordinate-temp.endNode.x)){
						temp.startNode.x=0.5*(temp.startNode.x+temp.endNode.x);
						for(std::vector<Record>::iterator p=activeRecordsOnSegment.begin();p!=activeRecordsOnSegment.end();++p){
							if((p->xCoordinate-temp.startNode.x)*(p->xCoordinate-temp.endNode.x)<=0)
								++num;
						}
						if(num<i->second)
							break;
						else{
							str.push_back('1');
						}
					}
					else{
						temp.endNode.x=0.5*(temp.startNode.x+temp.endNode.x);
						for(std::vector<Record>::iterator p=activeRecordsOnSegment.begin();p!=activeRecordsOnSegment.end();++p){
							if((p->xCoordinate-temp.startNode.x)*(p->xCoordinate-temp.endNode.x)<=0)
								++num;
						}
						if(num<i->second)
							break;
						else{
							str.push_back('0');
						}
					}
				}
				else{
					if(abs(i->first.yCoordinate-temp.startNode.y)>abs(i->first.yCoordinate-temp.endNode.y)){
						temp.startNode.y=0.5*(temp.startNode.y+temp.endNode.y);
						for(std::vector<Record>::iterator p=activeRecordsOnSegment.begin();p!=activeRecordsOnSegment.end();++p){
							if((p->yCoordinate-temp.startNode.y)*(p->yCoordinate-temp.endNode.y)<=0)
								++num;
						}
						if(num<i->second)
							break;
						else{
							str.push_back('1');
						}
					}
					else{
						temp.endNode.y=0.5*(temp.startNode.y+temp.endNode.y);
						for(std::vector<Record>::iterator p=activeRecordsOnSegment.begin();p!=activeRecordsOnSegment.end();++p){
							if((p->yCoordinate-temp.startNode.y)*(p->yCoordinate-temp.endNode.y)<=0)
								++num;
						}
						if(num<i->second)
							break;
						else{
							str.push_back('0');
						}
					}
				}
			}
			resultSegment[0].children=str;
			resultWithoutPredict.insert(std::make_pair(*i,resultSegment));
		}
		else{//如果当前路段不满足
			std::vector<Node> nodeTemp;
			nodeTemp.push_back(firstSegment.startNode);
			nodeTemp.push_back(firstSegment.endNode);
			while(activeUsers<i->second){
				RoadSegment newSegment,lastSegment;
				lastSegment=resultSegment[resultSegment.size()-1];
				if(mapNode.count(lastSegment.endNode)==1){
					Node tempNode=lastSegment.startNode;
					lastSegment.startNode=firstSegment.endNode;
					lastSegment.endNode=tempNode;
				}
				newSegment.startNode=lastSegment.endNode;
				std::multimap<Node,Node>::const_iterator findResult=mapNode.find(newSegment.startNode);
				std::multimap<Node,Node>::size_type entries =mapNode.count(newSegment.startNode);
				for(std::multimap<Node,Node>::size_type ct=0;ct!=entries;++ct,++findResult){
					if(std::find(nodeTemp.begin(),nodeTemp.end(),findResult->second)==nodeTemp.end()){
						newSegment.endNode=findResult->second;
						nodeTemp.push_back(findResult->second);
						break;
					}
				}
				if(newSegment.endNode.x==0.0&&newSegment.endNode.y==0.0){
					//std::cout<<"fail";
					resultSegment.erase(resultSegment.begin(),resultSegment.end());
					break;
				}
				resultSegment.push_back(newSegment);
				ComputerActiveRecords(activeRecordsOnSegment,activeRecordsCurrent,resultSegment[resultSegment.size()-1]);
				activeUsers=activeRecordsOnSegment.size();
			}
			if(activeUsers>i->second){
				int redundant=activeUsers-i->second;//多出的用户
				RoadSegment segment=resultSegment[resultSegment.size()-1];
				int flag=0;
				int breakFlag=0;
				std::string str="";
				while(true){
					++breakFlag;
					if(breakFlag>10){
						//str="";
						break;
					}
					if(segment.endNode.x!=segment.startNode.x){
						segment.endNode.x=0.5*(segment.endNode.x+segment.startNode.x);
						for(std::vector<Record>::iterator p=activeRecordsOnSegment.begin();p!=activeRecordsOnSegment.end();++p){
							if((p->xCoordinate-segment.endNode.x)*(p->xCoordinate-resultSegment[resultSegment.size()-1].endNode.x)<=0)
								++flag;
						}
						if(flag<redundant)
							str.push_back('0');
						else
							break;
					}
					else{
						segment.endNode.y=0.5*(segment.endNode.y+segment.startNode.y);
						for(std::vector<Record>::iterator p=activeRecordsOnSegment.begin();p!=activeRecordsOnSegment.end();++p){
							if((p->yCoordinate-segment.endNode.y)*(p->yCoordinate-resultSegment[resultSegment.size()-1].endNode.y)<=0)
								++flag;
						}
						if(flag<redundant)
							str.push_back('0');
						else
							break;
					}
				}
				resultSegment[resultSegment.size()-1].children=str;
			}
			resultWithoutPredict.insert(std::make_pair(*i,resultSegment));
		}
	}

}

void Predict(const std::map<Record,int>& goodRecord,
	const std::vector<RoadSegment>& vRoad,
	const std::multimap<Node,Node>& mapNode,
	const std::vector<Record>& activeRecordsCurrent,
	const std::vector<Record>& predictionRecordsPrevious,//用于预测的用户
	const std::vector<Record>& predictionRecordsCurrent,//用于判断预测是否准确
	std::map<std::pair<Record,int>,std::vector<RoadSegment>>& resultPredict//返回值
	)
{
	for(std::map<Record,int>::const_iterator i=goodRecord.begin();i!=goodRecord.end();++i){
		std::vector<RoadSegment> resultSegment;
		RoadSegment firstSegment=seekRoadSegment(i->first,vRoad);//定位当前路段
		resultSegment.push_back(firstSegment);
		std::vector<Record> activeRecordsOnSegment,prActiveRecordsOnSegment;
		ComputerActiveRecords(activeRecordsOnSegment,activeRecordsCurrent,firstSegment);
		int prNum=i->second-activeRecordsOnSegment.size();//还需要用户
		ComputerActiveRecords(prActiveRecordsOnSegment,predictionRecordsCurrent,firstSegment);
		/*if(prActiveRecordsOnSegment.size()>prNum)
			std::cout<<0;
		else
			std::cout<<1;*/;
	}
	std::vector<Record> temp=activeRecordsCurrent;
	for(std::vector<Record>::const_iterator p=predictionRecordsCurrent.begin();p!=predictionRecordsCurrent.end();++p)
		temp.push_back(*p);
	NoPredict(goodRecord, vRoad, mapNode, temp, resultPredict);
}

void SeletctGoodResult(
	const std::map<std::pair<Record,int>,std::vector<RoadSegment>>& resultWithoutPredict,
	std::map<std::pair<Record,int>,std::vector<RoadSegment>>& goodResult
	)//选取对比数据
{
	for( std::map<std::pair<Record,int>,std::vector<RoadSegment>>::const_iterator i=resultWithoutPredict.begin();i!=resultWithoutPredict.end();++i){
		if(i->second.size()==2||i->second.size()==1)
			goodResult.insert(*i);
	}

}

void GetGoodRecords(
	const std::map<std::pair<Record,int>,std::vector<RoadSegment>>& goodResult,
	std::map<Record,int>& goodRecord
	)
{
	for(std::map<std::pair<Record,int>,std::vector<RoadSegment>>::const_iterator i=goodResult.begin();i!=goodResult.end();++i)
		goodRecord.insert(i->first);
}
void Dis(
		double &len,
		const RoadSegment& roadSegment
		)
{
	
	double length=std::sqrt((roadSegment.startNode.x-roadSegment.endNode.x)*(roadSegment.startNode.x-roadSegment.endNode.x)
		+(roadSegment.startNode.y-roadSegment.endNode.y)*(roadSegment.startNode.y-roadSegment.endNode.y));
	std::string::size_type m=roadSegment.children.size();
	len+=length/std::pow(2.0,(double)m);
}

void ResultHandl(
	const std::map<std::pair<Record,int>,std::vector<RoadSegment>>& resultWithoutPredict,
	const std::map<std::pair<Record,int>,std::vector<RoadSegment>>& resultPredict
	)
{
	std::ofstream file("result.dat");
	std::map<std::pair<Record,int>,std::vector<RoadSegment>>::const_iterator pPredict=resultPredict.begin();
	std::map<std::pair<Record,int>,std::vector<RoadSegment>>::const_iterator pNoPre=resultWithoutPredict.begin();
	while(pPredict!=resultPredict.end()&&pNoPre!=resultWithoutPredict.end()){
		double lenPr=0.0,len=0.0;//总路长
		for(std::vector<RoadSegment>::const_iterator i=pPredict->second.begin();i!=pPredict->second.end();++i)
			Dis(lenPr,*i);
		for(std::vector<RoadSegment>::const_iterator i=pNoPre->second.begin();i!=pNoPre->second.end();++i)
			Dis(len,*i);
		double rate=(len-lenPr)/len;//路长减少比率
		if(!(rate<0||rate>1)){
			file<<pNoPre->first.first.bianHao<<'\t'<<pNoPre->first.first.time<<'\t'<<
				pNoPre->first.second<<'\t'<<len<<'\t'<<lenPr<<'\t'<<rate<<'\n';
			
		}
		++pPredict;
		++pNoPre;
	}
}
int main(){
std::vector<RoadSegment> vRoad;
std::multimap<Node,Node> mapNode;//通过Node找到与之相连的Node
initRoad(vRoad);
initNode(mapNode);
//test code
//Record record;
//record.xCoordinate=5533.0839104573715;
//record.yCoordinate=19256.70229850609;
//RoadSegment roadSegment=seekRoadSegment(record,vRoad);

//for(std::vector<RoadSegment>::const_iterator i=vRoad.begin();i!=vRoad.end();++i){
//	std::vector<Record> vRecord;
//	ComputerUserNumber(vRecord,*i,10);
//	std::cout<<vRecord.size();
//}


/*********************smimulation start******************/
int time=20;
std::vector<Record> activeRecordsCurrent;
std::vector<Record> activeRecordsPrevious;
std::vector<Record> predictionRecordsPrevious;//用于预测的用户
std::vector<Record> predictionRecordsCurrent;//用于判断预测是否准确
std::map<Record,int> requestCurrent;//请求用户
std::map<std::pair<Record,int>,std::vector<RoadSegment>> resultWithoutPredict;//无预测的方法返回的结果
std::map<std::pair<Record,int>,std::vector<RoadSegment>> resultPredict;//预测方法返回的结果
std::map<std::pair<Record,int>,std::vector<RoadSegment>> goodResult;
std::map<Record,int> goodRecord;//预测输入
ComputerActiveRecords(activeRecordsCurrent,activeRecordsPrevious,time);//得到当前时刻和上一时刻活跃记录
GetPredictionRecords(activeRecordsPrevious,activeRecordsCurrent,predictionRecordsPrevious,predictionRecordsCurrent);//得到上个时刻活跃，当前时刻不活跃的记录，作为为预测用户
InitRequestCurrent(activeRecordsCurrent.size(),requestCurrent,activeRecordsCurrent);
NoPredict(requestCurrent, vRoad, mapNode, activeRecordsCurrent, resultWithoutPredict);
SeletctGoodResult(resultWithoutPredict,goodResult);//选一些无预测的实验结果
GetGoodRecords(goodResult,goodRecord);//得到这些记录，作为预测的输入
Predict( goodRecord,vRoad, mapNode, activeRecordsCurrent, predictionRecordsPrevious, predictionRecordsCurrent, resultPredict);
ResultHandl(goodResult,resultPredict);
return 0;
}