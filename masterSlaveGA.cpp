
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <iterator>

using namespace std;
//capacity could be 70% favorable outcome
//capacity is the value that all threads commit to
#define num_threads 16
#define m_size 128


static bool check_fav(char* fav, char* a );
static void count_fav( int thread_index );
static void mutate(char** a,int index);
static void swap(char** a, char** b, int start);




struct thread_data {
    int thread_id;
    int local_count;
    char** fav;
};


char** m_space;
int chunk_size; //size of chunk
int n; // size of string length
char* fav; ///give this to the threads

int numBlocks;

struct thread_data* thread_data_array;

bool fav_capacity = false;

int main(int argc, char** argv) {
    
    if(argc != 4) {
        cout <<"usage: %s n chunck_size fav\n" <<  argv[0] << endl;
        return 0;
    }
    n = strtol(argv[1], NULL, 10);
    chunk_size = strtol(argv[2], NULL, 10);
    //TODO: is fav right?
    fav = argv[3];
    numBlocks = n/chunk_size;
    
    //malloc m_space
    m_space = (char**) malloc(sizeof(char*)*m_size*m_size);
    
    
    thread_data_array = (struct thread_data*) malloc (sizeof(struct thread_data)* num_threads);
    pthread_t threads[num_threads];
    int rc;
    pthread_attr_t attr;
    
    
    //initialize attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    //Initial state of the matrix: even rows are bits of size n all 0’s, size of n bits all 1’s.
    int i;
    for(int i =0; i<m_size*m_size; i++){
        m_space[i] = (char*)malloc((n*sizeof(char)));

        if(i%2 == 0) { //row is even
            
            for( int q = 0; q < n; q++) {
                    m_space[i][q] = '1';
                }
        } else {
            for(int q = 0; q < n; q++) {
                m_space[i][q] = '0';
                }
            }
    }
                                    
                                   
                                    
    while(!fav_capacity) {
        vector<char*> matingPool;
        vector<int>matingIndex;

        
        //copy over m_space into matingPool vector;
        
        for(int k=0; k<m_size *m_size; k++){
            //populate
            matingPool.push_back(m_space[k]);
            matingIndex.push_back(k);
        }
        
        
        int temp_size = m_size*m_size;
        
            for(int k =0; k<m_size/2 ; k++){
                                            
                //crossover - for all in mating pool, pick a random partner, switch one random block
                int firstIndex = rand()%(temp_size);
                int secondIndex = rand()%(temp_size);
                while(firstIndex==secondIndex) {
                    secondIndex = rand()%(temp_size);
                    }
            
                
                char*m1;
                char*m2;
                
                //Iterators
                vector<char*>::iterator it= matingPool.begin();
                vector<int>::iterator iter = matingIndex.begin();
                
                for( ; iter != matingIndex.end(); iter++)
                {
                    
                    if(*iter == firstIndex){
                        m1 = matingPool.at(it);
                    }
                    
                    if(*iter == secondIndex){
                        m2 == matingPool.at(it);
                    }
                    
                    it++;
                }
                //Recombination - save the new values determined by crossover
                int blockID = rand()%numBlocks;
                swap(&m1, &m2, blockID*chunk_size);
              
                vector<char*>::iterator it = matingPool.begin();
                for( ; it != matingPool.end(); it++)
                {
                    if(matingPool.at(&it) == m1){
                        matingPool.erase(it);
                    }
                    
                    if(matingPool.at(it) == m2){
                        matingPool.erase(it);
                    }

                }
                
                
                temp_size -= 2;
            
            }
                                        
        
            //Mutation - 10% chance to flip one random bit
            for (int i = 0; i < m_size*m_size; i++) {
                int chance = rand()%100;
                if(chance < 10) {
                    int bit = rand()%n;
                    mutate(m_size[i], bit);
                    }
            }
        
        
        
            //update capacity value by using slaves
            //split the m_size*m_size matrix into num_threads amount of threads
            for(int j = 0; j<num_threads; j++){
                    thread_data_array[j].thread_id = j;
                    thread_data_array[j].local_count = 0;
                    thread_data_array[j].fav = fav;
                    rc = pthread_create(&threads[j], NULL, count_fav, (void *) &thread_data_array[j]);
                            if(rc) {cout << "ERROR creating thread" << endl;}
            }
                                        
        //thread join
 
            for(int k = 0; k<num_threads; k++){
                rc = pthread_join(threads[k], NULL);
                if(rc) { cout << "ERROR joining thread" << endl;}
            }
        
        
                                        
            //after thread join, decide population number
            double total_fav_pop = 0;
            for(int i =0; i<num_threads; i++){
                    total_fav_pop += thread_data_array[i].local_count;
                }
                                        
            //total_fav_pop now has all of favorable people
                                        
            //decide capacity
            //70% favorable
            double population = m_size * m_size;
            double cap = total_fav_pop / population;
                                        
                if(cap >= .7) {
                        fav_capacity = true;
                }
        
            //inside while
        
            }
        //outside while inside main
        return 0;
                                    
        }
                                
                                    
                                    
                                    
                                    
        //recombination
        void swap(char** a, char** b, int start){
                char* temp = a;
                for(int i = start; i < start+chunk_size;i++) {
                            a[i] = b[i];
                            b[i] = temp[i];
                    }
        }
                                    
                                        
        void mutate(char** a,int index) {
                a[index] = ~a[index];
            }
                                        
                                        
        //THREAD FUNCTION
        void count_fav( int thread_index ) {
                int start = thread_index *(m_size * m_size ) / num_threads;
                int end = start + (m_size * m_size ) / num_threads;
                int local_count = 0;
            

                for(int i = start; i<end; i++){
                    //check favorability of m_space[i];
                    char*[chunk_size] my_string = m_space[i];
                                                
                    if(check_fav(my_string)) {
                        local_count++;
                        }
                    }
        }
        
        //finds favorable trait inside the bit string of length n
        bool check_fav(char* fav, char* a ){

                for(int j=0; j<n/chunk_size; j++){
                        int sameBits = 0;

                        for(int k=0; k<chunk_size; k++){
                                    int index = j * chunk_size + k;
                                    if(a[index] == fav[k]){
                                            sameBits++;
                                            }
                                        }
                                    if(sameBits == chunk_size){
                                                return true;
                                        }
                                    }
                                return false;
                            }
