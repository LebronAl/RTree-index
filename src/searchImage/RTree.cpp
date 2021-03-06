// RTree.cpp: 定义控制台应用程序的入口点。
//

// Test.cpp
//
// This is a direct port of the C version of the RTree test program.
//
#include"stdafx.h"
#include<string>
#include<fstream>
#include"RTree.h"
#include"readFile.h"
#include<time.h>
#include<math.h>
#include<map>
#include<algorithm>
using namespace std;

#define NDEBUG

vector<string> imageNameList_found;  //查询到的图片名字列表
map<string, double> Image; //将图片名字与其与查询点的距离关联

bool MySearchCallback(string* id,void* arg)
{
	imageNameList_found.push_back(*id);
	return true; // keep going
}

bool cmp(const string &a,const string &b)       //距离排序
{
	return Image[a] < Image[b];
}

int main()
{	
	srand(time(NULL));
	string feature_Filename = "LBP_feature.txt";
	string image_Filename = "imagelist.txt";
	vector<vector<double>> featureArray;
	vector<string> imageNameArray;  //所有的imagename
	readFeature(feature_Filename, featureArray);
	readImage(image_Filename, imageNameArray);

	map<string, int> imageName_number;                    //统计不同种类的图片的数量
	map<string, int>::iterator it;
	for (int i = 0; i < imageNameArray.size(); i++)          //加载图片种类
	{
		string a = imageNameArray[i];
		a.erase(9);
		it = imageName_number.find(a);
		if (it != imageName_number.end())
			imageName_number[a]++;
		else
			imageName_number.insert(pair<string, int>(a, 1));
	}
	
	const int feature = 64;                      //维数

	int n = featureArray.size();
	double **array_feature = new double*[n];
	for (int i = 0; i < n; i++)
		array_feature[i] = new double[feature];
	for (int i = 0; i < n; i++)
		for (int j = 0; j < feature; j++)
			array_feature[i][j] = featureArray[i][j];

	RTree<string*, double, feature, double> tree;


	for (int i = 0; i< n; i++)
	{
		tree.Insert(array_feature[i], array_feature[i], &imageNameArray[i]); // Note, all values including zero are fine in this version
	}

	double average = 0;   //准确率平均
	double average_ = 0;  //命中率平均

	//for (int i = 0; i <100; i++)
	//{
		double min[feature], max[feature];
		int hits = 0;
		int num = imageNameArray.size();
		cout << "请输入图片全称: ";
		string input_image;
		cin >> input_image;
		for (int i = 0; i < imageNameArray.size(); i++)
		{
			if (imageNameArray[i] == input_image)
				num = i;
		}
		if (num == imageNameArray.size())
		{
			cout << "图片不在数据库中！" << endl;
			system("pause");
			exit(-1);
		}
		//int num = rand() % n;
		vector<double> a = featureArray[num];
		for (int i = 0; i < feature; i++)                         //第一步大致提取
		{
			min[i] = 0.0;
			max[i] = a[i] * 5;
		}
		int nhits = tree.Search(min, max, MySearchCallback, NULL);


		for (int i = 0; i < imageNameList_found.size(); i++)          //加载图片种类
		{
			string temp = imageNameList_found[i];
			int m;
			for (m = 0; m < imageNameArray.size(); m++)
			{
				if (imageNameArray[m] == temp)
					break;
			}
			//double dis = 0;
			//for (int i = 0; i < feature; i++)
			//{
			//	dis += sqrt((a[i] / (double)n)*(featureArray[m][i] / (double)n));
			//}
			double dis_a = 0;             //欧氏距离度量
			double dis_f = 0;
			for (int i = 0; i < feature; i++)
			{
				dis_a += a[i] * a[i];
				dis_f += featureArray[m][i] * featureArray[m][i];
			}
			dis_a = sqrt(dis_a);
			dis_f = sqrt(dis_f);
			double dis = 0;
			for (int i = 0; i < feature; i++)
			{
				dis += sqrt((a[i] - featureArray[m][i])*(a[i] - featureArray[m][i]) / dis_a / dis_f);
			}
			//double dis = 0;              //直方图相似度量
			//double dis_a = 0;
			//for (int i = 0; i < feature; i++)
			//{
			//	dis += (a[i] < featureArray[m][i]) ? a[i] : featureArray[m][i];
			//	dis_a += a[i];
			//}
			//dis = dis / dis_a;
			Image.insert(pair<string, double>(temp, dis));
		}

		//int m;
		//for (m = 0; m < imageNameArray.size(); m++)
		//{
		//	if (imageNameArray[m] == imageNameArray[ranNum])
		//		break;
		//}
		//double dis_true = 0;
		//for (int i = 0; i < feature; i++)
		//{
		//	dis_true += sqrt((a[i] / (double)n)*(a[i] / (double)n));
		//}

		vector<string>::iterator it_;                                            //第二步仔细挑选
		for (it_ = imageNameList_found.begin(); it_ != imageNameList_found.end(); )
		{
			if (Image[*it_] > 1)                //欧氏距离为if(Image[*it] > 1)          巴氏距离为if(Image[*it] < dis_true - 2||Image[*it] > dis_true + 2)
			{                                    //直方图相似为if(Image[*it] < 0.55)   
				it_ = imageNameList_found.erase(it_);
			}
			else
			{
				it_++;
			}
		}

		sort(imageNameList_found.begin(), imageNameList_found.end(), cmp);

		for (int i = 0; i < imageNameList_found.size(); i++)
		{
			cout << "Hit data " << imageNameList_found[i] << ", ";
			cout << "Euclidean distance = " << Image[imageNameList_found[i]] << endl;
		}

		int hitNumber = 0;
		string temp = imageNameArray[num];
		string temp_test = temp;
		temp.erase(9);
		for (int i = 0; i < imageNameList_found.size(); i++)
		{
			string temp_ = imageNameList_found[i];
			temp_.erase(9);
			if (temp_ == temp)
				hitNumber++;
		}

		cout << "查出数据个数为" << imageNameList_found.size() << endl;
		cout << "其中正确个数为" << hitNumber << endl;
		cout << "正确率 = " << (double)100 * hitNumber / (double)imageNameList_found.size() << " % " << endl;
		cout << "命中率 = " << (double)100 * hitNumber / (double)imageName_number[temp] << " % " << endl;

		average += (double)100 * hitNumber / (double)imageNameList_found.size();
		average_ += (double)100 * hitNumber / (double)imageName_number[temp];

		imageNameList_found.clear();
		Image.clear();
	//}
	//cout << "平均正确率 = " << average /100<<"%"<< endl;
	//cout << "平均命中率 = " << average_ /100<< "%"<<endl;

	


	for (int i = 0; i < n; i++)
	{
		delete[] array_feature[i];
		array_feature[i] = NULL;
	}
	delete[] array_feature;
	array_feature = NULL;

	system("pause");
	
	return 0;
}