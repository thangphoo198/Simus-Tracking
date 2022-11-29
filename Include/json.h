/*
 * json.h
 *
 * Created: 8/28/2022 12:50:40 AM
 *  Author: hoang
 */ 


#ifndef JSON_H_
#define JSON_H_
char ktt(char inp);
char numb_value_in_json(char *json,int ln);
char get_key_in_json(char ind,char *jkey,char *key_ln,char *json,int j_ln);
char get_data_in_json(char ind,char *jvalue,char *value_ln,char *json,int j_ln);
char get_value_in_json(char *jkey,char key_ln,char *jvalue,char *value_ln,char *json,int j_ln);
char ss_string(char *str1,char ln1,char *str2,char ln2);
#endif /* JSON_H_ */