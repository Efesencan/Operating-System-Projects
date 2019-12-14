#include <iostream>
#include <pthread.h>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <time.h>
// Efe Sencan 25083
using namespace std;
// global variables
vector<vector<int> > matrix(2,vector<int>(50,0));
//int flag1 = 0,flag2 = 0,flag3 = 1;
int turn = 0;
int count = 0;
int c1 = 0,c2 = 0,c3 = 0;

void *firstAgency(void *param)
{
  while (true && count< 100) {
    int * paramPointer = (int *) param;
    while(count < 100 && (turn != 0));

    cout<<"Thread "<<1<<" enters critical region"<<endl;
    int first = rand() % 2;
    int second = rand() % 50;
    while(count < 100 && matrix[first][second] != 0){
      first = rand() % 2;
      second = rand() % 50;
    }
    matrix[first][second] = 1;
    c1++;
    count++;
    if(first == 0)
      cout<<"Seat Number "<< 2*(second + 1) - 1<<" is reserved by "<<*paramPointer<<endl;
    else{
      cout<<"Seat Number "<< 2*(second + 1) <<" is reserved by "<<*paramPointer<<endl;
    }
    cout<<"Thread "<<1<<" exists critical region"<<endl;
    turn = 1;
  }
}

void *secondAgency(void *param)
{
  while(true && count < 100){
    int * paramPointer = (int *) param;
    while(count < 100 && (turn != 1));
    if(count == 100)
      break;
    cout<<"Thread "<<*paramPointer<<" enters critical region"<<endl;
    int first = rand() % 2;
    int second = rand() % 50;
    while(count < 100 && matrix[first][second] != 0){
      first = rand() % 2;
      second = rand() % 50;
      //cout<<count<<endl;
      //cout<<first<<" "<<second<<endl;
    }
    matrix[first][second] = 2;
    //if(count < 100)
    c2++;
    count++;

    if(first == 0)
      cout<<"Seat Number "<< 2*(second + 1) - 1<<" is reserved by "<<*paramPointer<<endl;
    else{
      cout<<"Seat Number "<< 2*(second + 1) <<" is reserved by "<<*paramPointer<<endl;
    }
    cout<<"Thread "<<*paramPointer<<" exists critical region"<<endl;
    turn = 2;

  }
}

void *thirdAgency(void *param)
{
  while(true && count < 100){
    int * paramPointer = (int *) param;
    while(count < 100 && (turn != 2));
    if(count == 100)
      break;
    cout<<"Thread "<<*paramPointer<<" enters critical region"<<endl;
    int first = rand() % 2;
    int second = rand() % 50;
    while(count < 100 && matrix[first][second] != 0){
      first = rand() % 2;
      second = rand() % 50;
    }
    matrix[first][second] = 3;
    //if(count < 100)
    c3++;
    count++;

    if(first == 0)
      cout<<"Seat Number "<< 2*(second + 1) - 1<<" is reserved by "<<*paramPointer<<endl;
    else{
      cout<<"Seat Number "<< 2*(second + 1) <<" is reserved by "<<*paramPointer<<endl;
    }
    cout<<"Thread "<<*paramPointer<<" exists critical region"<<endl;
    turn = 0;
  }
}


int main(){
  srand(time(NULL));
  pthread_t  TravelAgency1,TravelAgency2,TravelAgency3;
  int id1 = 1;
  int id2 = 2;
  int id3 = 3;

  pthread_create( &TravelAgency1, NULL, firstAgency, (void*) &id1);
	pthread_create( &TravelAgency2, NULL, secondAgency, (void*) &id2);
  pthread_create( &TravelAgency3, NULL, thirdAgency, (void*) &id3);

  pthread_join(TravelAgency1, NULL);
  pthread_join(TravelAgency2, NULL);
  pthread_join(TravelAgency3, NULL);
  //cout<<"c1: "<<c1<<" c2: "<<c2<<" c3: "<<c3<<endl;
  for(int i  = 0; i<matrix.size(); i++){
    for(int j = 0; j<matrix[i].size(); j++){
      cout<<matrix[i][j];
    }
    cout<<endl;
  }
  return 0;
}
