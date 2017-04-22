#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <iostream>
#include <iterator>
#include <string.h>

using namespace std;
//capacity could be 70% favorable outcome
//capacity is the value that all threads commit to
#define num_threads 16
#define m_size 128


// g++ -lrt -lpthread -o masterslave masterSlaveGA.cpp
static bool check_fav(char* fav, char* a);
static void* count_fav( void* threadarg );
static void mutate(char* a,int index);
static void swap(char** a, char** b, int start);


struct thread_data {
    int thread_id;
    int local_count;
    char* fav;
};



char** m_space;
int chunk_size; //size of chunk
int n; // size of string length
char* fav; ///give this to the threads

int numBlocks;

bool fav_capacity = false;


// main function
int main(int argc, char** argv) 
{
    printf("before anythign in main \n");
    
    if(argc != 4) 
    {
        cout <<"usage: %s n chunck_size fav\n" <<  argv[0] << endl;
        return 0;
    }
    n = strtol(argv[1], NULL, 10);
    chunk_size = strtol(argv[2], NULL, 10);
    //TODO: is fav right?
   
   // int l;
   // for(l=0; l<chunk_size; l++){
    fav= argv[3];
  //  }
    
    printf("n=%d, chunk_size = %d, fav= %s \n", n, chunk_size, fav);
    
  //  fav[0] = argv[3];
    
    
    
    
    numBlocks = n/chunk_size;
    
    //malloc m_space
    m_space = (char**)malloc(sizeof(char*)*m_size*m_size);
    
    //pthreads
    struct thread_data thread_data_array[num_threads];
    pthread_t threads[num_threads];
    int rc;
    pthread_attr_t attr;
    
    printf("made it to after init \n");
    
    //initialize attribute
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    //Initial state of the matrix: even rows are bits of size n all 0’s, size of n bits all 1’s.
    char** temp = m_space;
    int i = 0;
    while(temp < m_space+(m_size*m_size))
    {
       *temp = (char*)malloc((n+1)*sizeof(char));
       // printf("inside for \n");
        if(i%2 == 0) 
        { //row is even
           
            for( int q = 0; q < n; q++)
            {   //printf("inside inner for from 0 to n \n");
                *temp += '1';
            }
            *temp += '\0';
          //  cout << m_space[i] << endl;
        } 
        else 
        {
            for(int q = 0; q < n; q++) 
            {
                *temp += '0';
            }
          //  cout << m_space[i] << endl;
            *temp += '\0';
        }
        
        temp++;
        i++;
    }
    
    

    
    
    printf("finished assigning 0 and 1 to matrix \n");
    while(!fav_capacity) 
    {
        vector<char*> matingPool;
        vector<int>matingIndex;

        //copy over m_space into matingPool vector;        
        for(int k=0; k<m_size *m_size; k++)
        {
            //populate
            //std::string temp(m_space[k]);
            
           // cout << temp << endl;
            printf("mspace[k] = %s \n",*(m_space + k) );
            matingPool.push_back(*(m_space + k));
            matingIndex.push_back(k);
        }
        printf("after pushback \n");
        for(int k =0; k<m_size*m_size/2 ; k++)
        {
            
            printf("k= %d \n", k);
            printf("in for mating \n");
            //crossover - for all in mating pool, pick a random partner, switch one random block
            int firstIndex = rand()%(matingPool.size());
            int secondIndex = rand()%(matingPool.size());
            while(firstIndex==secondIndex) 
            {
                secondIndex = rand()%(matingPool.size());
            }
            char*m1;
            char*m2;
            
            printf("first index = %d , second index = %d \n", firstIndex, secondIndex);
            
            
            //Iterators
            vector<char*>::iterator it= matingPool.begin();
            vector<int>::iterator iter = matingIndex.begin();
            
            for( ; iter != matingIndex.end(); iter++)
            {
              //  printf("inside mting index \n");
                
                if(*iter == firstIndex)
                {  printf("found first index \n");
                    m1 = *it;
                }
                
                if(*iter == secondIndex)
                {
                    printf("found second index \n");
                    m2 = *it;
                }
                
                it++;
            }

            printf("before recom \n");
            
            //Recombination - save the new values determined by crossover
            int blockID = rand()%numBlocks;
            swap(&m1, &m2, (blockID*chunk_size));
          
            printf("outside of swap \n");
            vector<char*>::iterator ita = matingPool.begin();
            printf("ita begin = %s \n", *matingPool.begin());
            printf("ita end = %s \n", *matingPool.end());
              printf("iterator set to begin  \n");
            for( ; ita != matingPool.end(); ita++)
            {  printf("inside for mating pool \n");
                if(*ita == m1) // this should work okay im not sure yet
                {
                    printf("found m1 \n");
                    matingPool.erase(ita);
                    break;
             
                }
                
            }
            
            
            ita = matingPool.begin();
            //erase m2
            for( ; ita != matingPool.end(); ita++)
            {  //printf("inside for mating pool \n");

                if((*ita) == m2)
                {   printf("found m2 \n");
                    matingPool.erase(ita);
                    break;
                }
            }
            
           // while(1);
            
        }//end of mating for loop
        
        printf("before mutation done with recom \n");
        
        //Mutation - 10% chance to flip one random bit
        for (int i = 0; i < m_size*m_size; i++) 
        {
            int chance = rand()%100;
            if(chance < 10) 
            {
                int bit = rand()%n;
                mutate(m_space[i], bit);
            }
        }
        
        
        
        //update capacity value by using slaves
        //split the m_size*m_size matrix into num_threads amount of threads
        for(int j = 0; j<num_threads; j++)
        {
            thread_data_array[j].thread_id = j;
            thread_data_array[j].local_count = 0;
            thread_data_array[j].fav = fav;
            rc = pthread_create(&threads[j], NULL, count_fav, (void *) &thread_data_array[j]);
            if(rc) 
            {
                cout << "ERROR creating thread" << endl;
            }
        }
                                        
        //thread join
 
        for(int k = 0; k<num_threads; k++)
        {
            rc = pthread_join(threads[k], NULL);
           if(rc) 
            {
                cout << "ERROR joining thread" << endl;
            }
        }
                                     
        //after thread join, decide population number
        double total_fav_pop = 0;
        for(int i =0; i<num_threads; i++)
        {
            total_fav_pop += thread_data_array[i].local_count;
        }
                                    
        //total_fav_pop now has all of favorable people
                                    
        //decide capacity
        //70% favorable
        double population = m_size * m_size;
        double cap = total_fav_pop / population;
                                    
        if(cap >= .7) 
        {
            fav_capacity = true;
        }
        
            //inside while
        
    }
        //outside while inside main
    
    
    //print results

    for(i = 0; i<m_size*m_size; i++){
        printf("%s", m_space[i]);
        if(i%32 == 0) printf("\n");
    }
    
    
    
    return 0; 
}
                                                                  
// functions/////////
//recombination
void swap(char** a, char** b, int start)
{
    char** temp = a;
    for(int i = start; i < start+chunk_size;i++) 
    {
        a[i] = b[i];
        b[i] = temp[i];
    }
}
                                                                           
void mutate(char* a, int index) 
{
    a[index] = ~a[index];
}
                                                                               
//THREAD FUNCTION
void* count_fav( void* threadarg )
{
    struct thread_data * mydata;
    mydata = (struct thread_data*) threadarg;

    int thread_index = mydata->thread_id; // i hope this is correct and works the same as before

    int start = thread_index *(m_size * m_size ) / num_threads;
    int end = start + (m_size * m_size ) / num_threads -1 ; // should be correct
    int local_count = 0;

    for(int i = start; i<end; i++)
    {
        //check favorability of m_space[i];
        char* favor = mydata->fav; // is this also going to work?
                                    
        if(check_fav(favor, m_space[i])) 
        {
            local_count++;
        }
    }
    mydata->local_count = local_count; // assign the local count value to the thread?
     pthread_exit(NULL);
}
        
//finds favorable trait inside the bit string of length n
bool check_fav(char* fav, char* a )
{
    for(int j=0; j<n/chunk_size; j++)
    {
        int sameBits = 0;

        for(int k=0; k<chunk_size; k++)
        {
            int index = j * chunk_size + k;
            if(a[index] == fav[k])
            {
                sameBits++;
            }
        }
        if(sameBits == chunk_size)
        {
            return true;
        }
    }
    return false;
}
