// #include<iostream>
// #include<cstring>
// #include<algorithm>
// #include<cmath>
// #include<map>
// #include<iterator>
// #include<vector>
// using namespace std;
// map<string,int> dict;
// struct node{
// 	string str;
// 	int num;
// };
// struct node nod;
// bool cmp(struct node a,struct node b)
// {
// 	if(a.num>b.num)return true;
// 	else if(a.num==b.num)return a.str<b.str;
// 	else return false;
// }
// int main()
// {
// 	char c;
// 	string str; 
// 	while(scanf("%c",&c)!=EOF)
// 	{
// 		if((c>='a'&&c<='z')||(c>='0'&&c<='9')||(c>='A'&&c<='Z')||(c=='_'))
// 		{
// 			if(c>='A'&&c<='Z') //将大写字母转换为小写
// 			{
// 			   c=c-'A'+'a';
// 			}
// 			if(str.length()<15) //只要没超过15个字母这个单词就可以继续存
// 			{
// 				str+=c;
// 			}
// 		}
// 		else if(c=='#'||str.length()>0)
// 		{
// 			if(!dict[str]){  //如果map不存在这个单词
// 				dict[str]=1;
// 			}
// 			else{            //map已经存了这个单词
// 				dict[str]++;
// 			}
// 			str.clear();
// 			if(c=='#')break;
// 		}
// 	}
// 	map<string,int>::iterator it; 
// 	vector<node> v;
//     //将存到map中的单词拿出来放到vector中，便于排序输出出现次数多的单词。
// 	for(it=dict.begin();it!=dict.end();it++)
// 	{
// 		if(it->first.length()>0)
// 		{
// 			nod.str=it->first;
// 			nod.num=it->second;
// 			v.push_back(nod); 
// 		}
// 	}
// 	sort(v.begin(),v.end(),cmp); //排序
// 	int count=v.size()*0.1;
// 	cout<<v.size()<<endl;
// 	for(int i=0;i<count;i++)
// 	{
// 		cout<<v[i].num<<":"<<v[i].str<<endl;
// 	}
// }



#include<bits/stdc++.h>
#define p pair<string,int>  //定义了一个pair p 
using namespace std;
bool cmp(p a,p b)
{
    if(a.second>b.second)   //先以单词数量从大到小排序 
        return true;
        
    if(a.second==b.second)  //如果数量相等 
        if(a.first<b.first) //再以字典序升序排序 
            return true;
            
    return false;
}
int main()
{
    map<string,int> ma;
    map<string,int>::iterator it; 
    vector<p> v;            //vector为pair类型
    string s;
    char ch;
    while(1) 
    {
        ch=getchar();       //用getchar暂存字符 
        if((ch>='a'&&ch<='z')||(ch>='A'&&ch<='Z')||(ch>='0'&&ch<='9')||(ch=='_'))
        {
            if(s.size()<=14)  
            {
                if(ch>='A'&&ch<='Z')   //如果是大写字母，则变成小写 
                    ch+=32;
                s+=ch;                 //将字符放入string s 
            }
        }
        else
        {
            if(s.size()>0) 
                ma[s]++;               //将字符串存到map中 
            s.clear();
        }
        if(ch=='#')break;
    }
    for(it=ma.begin();it!=ma.end();it++)
    {
        v.push_back(p(it->first,it->second));   //将map元素赋值到p，p作为中转器赋值到vector 
    }
    sort(v.begin(),v.end(),cmp);       //用vector排序！！！！ 
    cout<<ma.size()<<endl;             //用map的size计算不同单词数量 
    int cnt=(int)(ma.size()*0.1);      //前10%的单词
     
    for(int i=0;i<cnt;i++)
        cout<<v[i].second<<':'<<v[i].first<<endl;
    return 0;
}