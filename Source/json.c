/*
 * json.c
 *
 * Created: 8/27/2022 10:16:53 AM
 *  Author: hoang
 */ 
#include "json.h"
//#include "forESP32.h"

char ktt(char inp)
{
	char kq=0;
	// neu l� +-*
	if(inp==43||inp==42||inp==45)
	{
		kq=1;
	}else
	{
		//neu l� _ hoac \ //
		if(inp==95||inp==92)
		{
			kq=1;
		}else
		{
			//neu l� so
			if((inp>39)&&(inp<58))
			{
				kq=1;
			}else
			{
				//neu la chu hoa
				if((inp>64)&&(inp<91))
				{
					kq=1;
				}else
				{
					//neu la chu thuong
					if((inp>96)&&(inp<123))
					{
						kq=1;
					}
				}
			}
		}
	}
	return kq;
}
char numb_value_in_json(char *json,int ln)
{
	//t�m so : trong chuoi
	int i;
	char kq=0;
	for(i=0;i<ln;i++)
	{
		if(json[i]==':')kq++;
	}
	return kq;
}
char get_key_in_json(char ind,char *jkey,char *key_ln,char *json,int j_ln)
{
	int i, j, vt, key_start, key_end;
	char kq=0;
	//B1 tim v? tr� d?u : theo ind
	for(i=0;i<j_ln;i++)
	{
		if(json[i]==':')kq++;
		if(kq==ind)
		{
			vt=i;
			break;
		}
	}
	//B2 t�m v? tr� key
	//: error
	if(vt==0||vt==(j_ln-1))return 0;
	//: ok
	key_end=-1;
	key_start=-1;
	for(j=vt-1;j>=0;j--)
	{
		if((ktt(json[j])==1)&&(key_end==-1))
		{
			key_end=j;
		}else
		{
			if((ktt(json[j])==0)&&(key_end>-1))
			{
				key_start=j+1;
				break;
			}
		}
	}
	//B3 ghi l?i jkey
	//neu co ky tu
	if(key_end>=0)
	{
		//neu key_start==1 tuc chi co 1 ky tu
		if(key_start==-1)key_start=0;
		i=0;
		for(j=key_start;j<key_end+1;j++)
		{
			jkey[i]=json[j];
			i++;
		}
		*key_ln=i;
		kq=1;
		
	}
	
	return kq;
}
char get_data_in_json(char ind,char *jvalue,char *value_ln,char *json,int j_ln)
{
	int i, j, vt, key_start, key_end;
	char kq=0;
	//B1 tim v? tr� d?u : theo ind
	for(i=0;i<j_ln;i++)
	{
		if(json[i]==':')kq++;
		if(kq==ind)
		{
			vt=i;
			break;
		}
	}
	//B4 t�m v? tr� value
	//: error
	if(vt==0||vt==(j_ln-1))return 0;
	//: ok
	key_end=-1;
	key_start=-1;
	for(j=vt+1;j<j_ln;j++)
	{
		if((ktt(json[j])==1)&&(key_start==-1))
		{
			key_start=j;
		}else
		{
			if((ktt(json[j])==0)&&(key_start>-1))
			{
				key_end=j-1;
				break;
			}
		}
	}
	//B5 ghi l?i jvalue
	//neu co ky tu jvalue
	if(key_start>=0)
	{
		//neu key_start==1 tuc chi co 1 ky tu
		if(key_end==-1)key_end=j_ln-1;
		i=0;
		for(j=key_start;j<key_end+1;j++)
		{
			jvalue[i]=json[j];
			i++;
		}
		*value_ln=i;
		kq=1;
	}
	return kq;
}
char get_value_in_json(char *jkey,char key_ln,char *jvalue,char *value_ln,char *json,int j_ln)
{
	char numb_key,i,j,ind;
	char get_key[10];
	char get_key_ln;
	char khop=0;
	char kq=0;
	ind=0;
	numb_key=numb_value_in_json(json,j_ln);
	if(numb_key>0)
	{
		//tim ind cua key
		for(i=1;i<numb_key+1;i++)
		{
			if(get_key_in_json(i,get_key,&get_key_ln,json,j_ln)==1)
			{
				if(key_ln==get_key_ln)
				{
					khop=1;
					for(j=0;j<key_ln;j++)
					{
						if(get_key[j]!=jkey[j])
						{
							khop=0;
							break;
						}
					}
					if(khop==1)
					{
						ind=i;
						break;
					}
				}
			}
		}
		//da co duoc ind
		if(ind>0)
		{
			if(get_data_in_json(i,jvalue,value_ln,json,j_ln)==1)
			{
				kq=1;
			}
		}
	}
	return kq;
}
char ss_string(char *str1,char ln1,char *str2,char ln2)
{
	char kq=1;
	char i;
	if(ln1==ln2)
	{
		for(i=0;i<ln1;i++)
		{
			if(str1[i]!=str2[i])
			{
				kq=0;
				break;
			}
		}
	}else
	{
		kq=0;
	}
	return kq;
};