#include "sha1.hpp"
//#include "genMerkle.cpp"
//#include "HORS.cpp"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <zmq.hpp>
#include <math.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n) Sleep(n)
#endif

using namespace std;

struct hexVal{
	char hex[SHA1_HEX_SIZE];
	int k;
};

struct stringVal {
	char* str;
	int k;	
};

void gen_merkle_tree ( vector< vector<hexVal> > &gen_merkle_tree, vector<hexVal> leafs){

	//initalize tree parameters
	int n_leafs = leafs.size();
	int num_nodes = (2 * n_leafs) - 1;
	int num_levels = (log(num_nodes) / log(2)) + 1;
	
	vector< vector<hexVal> > merkle_tree;
	int leaf_iter = n_leafs;
	
	//intialize merkle tree levels
	for(int i=0; i < num_levels; i++){
	
		vector< hexVal > level;		
		merkle_tree.push_back(level);
	
	}
	
	cout<<"merk tree levels populated..."<<endl;
	
	//initialize bottom level
	for(int i=0; i <n_leafs; i++){
		
		//hex i-th message value, then add to bottom level
		hexVal hex;
		sha1(leafs[i].hex).finalize().print_hex(hex.hex);
		
		//add hex to tree bottom level
		merkle_tree[0].push_back(hex);
		cout<<merkle_tree[0][i].hex<<" added at "<<i<<endl;
	
	}

	cout<<"bottom level populated with size "<<merkle_tree[0].size()<<endl;
	
	for(int i=0; i < num_levels - 1; i++){
		int counter = 0;
		cout<<"leaf_iter = "<<leaf_iter<<endl;

		for(int a = 0; a < leaf_iter; a+=2){
			
			//create holder variables for
			//concatenated string
			//hexed concatenated string
			char concat[SHA1_HEX_SIZE*2];
			hexVal hex;
			
			//concantenate adjacent nodes
			strcpy(concat,merkle_tree[i][a].hex);
			cout<<"accessed a = "<<a<<endl;
			
			strcat(concat,merkle_tree[i][a+1].hex);
			cout<<"accessed a = "<<a+1<<endl;

			cout<<"concat nodes = "<<concat<<endl;	
			//hex concat
			sha1(concat).finalize().print_hex(hex.hex);
			cout<<"hash   = "<<hex.hex<<endl;
			//add hash to next level
			merkle_tree[i+1].push_back(hex);
			counter++;
				
			
		}
		//divide number of leafs /2 to determine
		//number of nodes in next level
		cout<<"level "<<i+1<<" populated."<<endl;

		leaf_iter /= 2;
	
	}
	cout<<"tree fully populated."<<endl;


	gen_merkle_tree =  merkle_tree;
}


vector <hexVal> gen_merk_values(vector< vector<hexVal> > merkle_tree, char leaf[SHA1_HEX_SIZE]){
	
	//find occurrence of leaf on the tree
	int index = 1000000;

	char hexLeaf[SHA1_HEX_SIZE];
	sha1(leaf).finalize().print_hex(hexLeaf);

	vector<hexVal> verify_leafs;

	char empty[SHA1_HEX_SIZE] = "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE";
	char leftN[SHA1_HEX_SIZE] = "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL";
	char rightN[SHA1_HEX_SIZE]= "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR";
	
	
	for(int i=0; i < merkle_tree[0].size(); i++){
		
		if(strcmp(hexLeaf,merkle_tree[0][i].hex) == 0){
		
			index = i;
			break;		

		}
		
	}
	
	//if hash did not belong to merkle tree first layer
	if(index == 1000000){
		
		hexVal hexE;
		strcpy(hexE.hex,empty);
		verify_leafs.push_back(hexE);
		return verify_leafs;
	}
	
	hexVal hexR,hexL;
	strcpy( hexR.hex, rightN);
	strcpy( hexL.hex, leftN);
	cout<<hexR.hex<<endl;
	cout<<hexL.hex<<endl;
			
	

	// else continue contruct root value from specific node
	int index_set = index+1;
	
	for(int i=0; i < merkle_tree.size(); i++){

		cout<<"index_set = "<<index_set<<endl;	
		
		if(i == merkle_tree.size()-1){
			verify_leafs.push_back(merkle_tree[i][0]);
		
		}
		
		else{
		
		if(index_set % 2 == 0){
			verify_leafs.push_back(hexR);
			verify_leafs.push_back(merkle_tree[i][(index_set -1)-1]);
		}
	
		if(index_set % 2 != 0){
			verify_leafs.push_back(hexL);
			verify_leafs.push_back(merkle_tree[i][(index_set +1)-1]);
		}
		
		//verify_leafs.push_back(merkle_tree[i][index_set-1]);
		double int_buffer = ceil(index_set/2.0);
		index_set = (int)int_buffer;
		
		}
	}	
	
	return verify_leafs;

}

int verify_tree(vector< vector<hexVal> > merkle_tree, char leaf [SHA1_HEX_SIZE] ){

	vector<hexVal> verify_leafs;
	verify_leafs  = gen_merk_values(merkle_tree,leaf);
	char hex[SHA1_HEX_SIZE];
	sha1(leaf).finalize().print_hex(hex);	


	char empty[SHA1_HEX_SIZE] = "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE";
	char leftN[SHA1_HEX_SIZE] = "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL";
	char rightN[SHA1_HEX_SIZE]= "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR";
	
	cout<<verify_leafs.size()<<endl;	
	for(int i=0; i < verify_leafs.size(); i++){
		cout<<verify_leafs[i].hex<<endl;	
		
	}
		
		
	hexVal hexR,hexL,hexE;
	strcpy( hexR.hex, rightN);
	strcpy( hexL.hex, leftN);
	strcpy( hexE.hex, empty);
	int count = 0;
	
	for(int i=0; i < verify_leafs.size()-1; i+=2){
		char concat[SHA1_HEX_SIZE * 2];
		char hexCheck[SHA1_HEX_SIZE];
		//char hex_res[SHA1_HEX_SIZE];
		cout<<"pass "<<count<<" of verification..."<<endl;
		cout<<hex<<endl;
			
		if( strcmp(verify_leafs[i].hex,hexR.hex) != 0){
			strcpy(concat,hex);
			strcat(concat,verify_leafs[i+1].hex);
			cout<<concat<<endl;
		
		}
		
		if( strcmp(verify_leafs[i].hex,hexR.hex) == 0){
			strcpy(concat,verify_leafs[i+1].hex);
			strcat(concat,hex);
			cout<<concat<<endl;
		
		}
		sha1(concat).finalize().print_hex(hexCheck);
		strcpy(hex,hexCheck);
		count++;
		//cout<<hexCheck<<endl;	
	}
	
	
	return strcmp(hex,merkle_tree[merkle_tree.size()-1][0].hex) == 0 ;
	

}


char* random_string(int length){
	static char charset[] = "10";
	char* random_string = NULL;

	if(length){
		random_string = (char*)malloc(sizeof(char)*(length+1));

	if(random_string) {
		for(int i=0; i<length; i++){
			int key = rand() % (int)(sizeof(charset)-1);
			random_string[i] = charset[key];
      		}
      		random_string[length] = '\0';
    	}
	}
 	//cout<<"random string generated: "; 
	//cout<<random_string<<endl;
	return random_string;
}

char* string_to_binary(char* s){
	if(s == NULL) return 0;
	size_t len = strlen(s);
	char *binary = (char*)malloc(len*8 + 1);
	binary[0] = '\0';
	for(size_t i = 0; i < len; ++i) {
        	char ch = s[i];
        	for(int j = 7; j >= 0; --j){
            		if(ch & (1 << j)) {
                		strcat(binary,"1");
            		} 
			else {
                		strcat(binary,"0");
            		}
        	}
    	}
    	return binary;
}


int btoi(char* s){
	register unsigned char *p = (unsigned char *)( s );
  	register unsigned int r = 0;

  	while(p && *p){
    		r <<=1;
    		r += (unsigned int)((*p++)& 0x01);
  	}
  	return (int)r;
}

vector< stringVal > split_string(char* string, int l){
	
	vector<stringVal> returnVec;
	
	for(int i=0; i < strlen(string) / l; i++){
		stringVal str;
		char splitStr[l];
		
		for(int a=0; a < l; a++){
				
			splitStr[a] = string[i+a];
		
		}
		strcpy(str.str,splitStr);
		returnVec.push_back(str);
	
	}	
	
	return returnVec;
	
}

void gen_keys(vector < hexVal > &pub_key, vector <stringVal> &sec_key ,int l, int k, int t){
	
	cout<<"generating keys..."<<endl;
	for(int i=0; i < t; i++){
		hexVal hex;
		stringVal sVal;
		char str[l];
		
		if( i==0){
		
			hex.k = k;
			sVal.k = k;
			cout<<"k copied...."<<endl;
		}
		
		//memset(&str,'\0',l);
		sVal.str = (char*) malloc(l+1);
		memset(sVal.str,'\0',l+1);	
		strcpy(str,random_string(l));
		
		//cout<<"random string "<<str<<" saved"<<endl;
			
		strncpy(sVal.str,str,l);
		//cout<<"key copied to stringVal struct"<<endl;	
		sec_key.push_back(sVal);
		//cout<<"key pushed back to secret key array"<<endl;

		sha1(str).finalize().print_hex(hex.hex);
		pub_key.push_back(hex);
		
		//cout<<"gen_key loop "<<i+1<<endl;	
		
	}
	
	cout<<"keys generated..."<<endl;

}


vector <stringVal > sign(char* m, vector< stringVal> sec_key, int t){
	
	hexVal hex;
	char* bin_string;
	sha1(m).finalize().print_hex(hex.hex);
	cout<<"hash sha1 val = "<<hex.hex<<endl;	
	size_t len = strlen(hex.hex);
	bin_string = (char*) malloc(len*8 +1);
	strcpy(bin_string,string_to_binary(hex.hex));
	
	cout<<"sign check 1"<<endl;
	int length = log(t)/log(2);
	
	vector<stringVal> signStrings;
	signStrings = split_string(bin_string, length);
	
	vector<stringVal> signature;
		
	for(int i=0; i < signStrings.size(); i++){
		int bin;
		char* bString;
		len = strlen(signStrings[i].str);
		bString = (char *) malloc(len*8 +1);
	
		strcpy(bString,string_to_binary(signStrings[i].str));
		cout<<bString<<endl;
		bin = btoi(bString);
		cout<<bin<<endl;	
		signature.push_back(sec_key[bin]);
		free(bString);	
		//free(bin);
	
	
	}
	cout<<signature.size()<<endl;
	cout<<sec_key.size()<<endl;	
	
	return signature;
	
	

}




int main(){
	
	int n_messages;
	int l = 128;
	int k = 32;
	int t = 1024;

	cout<<"input the number of messages to be signed: ";
	cin >> n_messages;
	
	vector< vector < hexVal > > merkle_tree;
	vector< vector < hexVal > > pub_keys;
	vector< vector < stringVal > > secret_keys;
	vector< vector < stringVal > > signatures;
	vector<  hexVal > leafs;
	for(int i=0; i < n_messages; i++){
		
		char m[SHA1_HEX_SIZE];
		hexVal hex;	
		for(int a=0; a < SHA1_HEX_SIZE; a++){
			m[a] = 'H';
		
		}
		vector < hexVal> public_key;
		vector <stringVal> secret_key;
		gen_keys(public_key,secret_key,l,k,t);
		
		pub_keys.push_back(public_key);
		secret_keys.push_back(secret_key);
		
		vector <stringVal> signature;
		signature = sign(m,secret_key,t);
		signatures.push_back(signature);
	
		strcpy(hex.hex,m);
		leafs.push_back(hex);
		
	
	
	}
	cout<<"check 1"<<endl;	
	vector <hexVal> in_keys;
	//generate merkle tree from public keys by pairs
	for(int i=0; i < pub_keys.size(); i++){
		
		for(int a=0; a < pub_keys[i].size(); a++){
			in_keys.push_back(pub_keys[i][a]);
			//cout<<i<<endl;
		
		}
		cout<<"loop check 1"<<endl;
		
	
	}
	
	gen_merkle_tree(merkle_tree,in_keys);
	char hex[SHA1_HEX_SIZE];
	sha1(secret_keys[0][0].str).finalize().print_hex(hex);
	cout<<verify_tree(merkle_tree,hex)<<endl;	
	
	
	
	return 0;
}


