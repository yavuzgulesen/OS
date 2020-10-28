//ygulesen 23664
#include <iostream>
#include <pthread.h>
#include <string>
#include <cstdlib>
using namespace std;

int seats[2][50];
int turn = 0;

bool isFull(){
	for(int i=0; i<2; i++){
		for(int j=0; j<50; j++){
			if(seats[i][j]==0){
				return false;
			}}}
	return true;
}

void *res(void * param){
	int * tid = (int *) param;
	while(!isFull()){
		if(isFull())
			break;
		while(turn != (*tid-1));	//busy waiting
		if(!isFull())
			cout << "TravelAgency " << *tid <<" entered the shared memory.\n";
		int random = rand()%100+1; // random is an element of [1,101]
		int row = (random+1) % 2;	// row = 0 if random is odd, row = 1 if random is even
		int column = ((random+1)/2)-1;	
		if(seats[row][column] == 0){ //if empty, then mark that seat
			seats[row][column] = *tid;
			cout << "TravelAgency "<< *tid << " reserved Seat Number "<< random << ".\n";
			cout << "TravelAgency " << *tid << " exit the shared memory.\n";
		}
		else if(seats[row][column] == *tid){ //if filled by itself before, find an empty seat 
			while(seats[row][column]!=0 && !isFull()){
				int random = rand()%100+1;
				int row = (random+1) % 2;
				int column = ((random+1)/2)-1;
				if(seats[row][column] == 0){
					seats[row][column] = *tid;
					cout << "TravelAgency "<< *tid <<" reserved Seat Number " << random << ".\n";
					cout << "TravelAgency " << *tid << " exit the shared area.\n"; //if found empty seat, break the while
					break;
				}
			}

		}
		else if(seats[row][column]!=0 && seats[row][column]!= *tid){ //if filled by another, find an empty seat
			while(seats[row][column]!=0 && seats[row][column]!=*tid && !isFull()){
				int random = rand()%100+1;
				int row = (random+1) % 2;
				int column = ((random+1)/2)-1;
				if(seats[row][column] == 0){ //if found an empty seat, break the while
					seats[row][column] = *tid;
					cout << "TravelAgency " << *tid<<" reserved Seat Number " << random << ".\n";
					cout << "TravelAgency " << *tid <<" exit the shared area.\n";
					break;
				}
			}
		}//update turn as you bought ticket
		// turn is an element of {0, 1, 2} => take modulo 3 after incrementing
		turn = ((turn+1) %3);
	}
}

int main(){

	for(int i=0; i<2; i++){
		for(int j=0; j<50; j++){
			seats[i][j] = 0;
		}
	}

	int id1 = 1;
	int id2 = 2;
	int id3 = 3;
	pthread_t t1, t2, t3;
	pthread_create(&t1, NULL, res, (void*) &id1);
	pthread_create(&t2, NULL, res, (void*) &id2);
	pthread_create(&t3, NULL, res, (void*) &id3);

	while(!isFull());

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	cout << endl<< "Seats (1-3-5-7-...-99):" << endl; //first row of seats
	cout << "|";
	for(int p=0; p<50; p++){
		cout << seats[0][p] << "|";
	}
	cout << endl << endl;
	cout << endl << "Seats (2-4-6-8-...-100):" << endl;//second row of seats
	cout << "|";
	for(int p=0; p<50; p++){
		cout << seats[1][p] << "|";
	}
	cout << endl << endl;
	return 0;
}
