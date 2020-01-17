//
//  main.cpp
//  A Simple Model of Flexible Working Memory
//
//  Created by Mahdi Heidarpoor on 10/8/19.
//  Copyright © 2019 Mahdi Heidarpoor. All rights reserved.
//

#include <iostream>
#include <math.h>
#include <time.h>

#define _USE_MATH_DEFINE

using namespace std;

//  ----------------------------------------------      PROGRAM     --------------------------------------------------------
const long double tao=10; // time constant 10ms in paper
//  -----------------------------------------------    FUNCTIONS   ---------------------------------------------------------
//  --------------------------------------------    Firerate function   ----------------------------------------------------
long double fire_rate_function(long double x)
{
    long double y=0;
    y=(long double)(1/tao)*(0.4)*(1+(long double)tanh(0.4*x-3));
    return y;
}

//  --------------------------------------------    Spiking function    ----------------------------------------------------
const long double poisson_dis=13.0066659551;
long double spiking_function(long double x)
{
    long double temp2=poisson_dis*(1-(long double)exp(-x));
    long double r=(long double)rand()/RAND_MAX;
    if(r<temp2) return 1;
    return 0;
}

//  --------------------------------------------    Synaps function    -----------------------------------------------------
long double synaps_function(int t,long double* x)
{
    long double y=0;
    long double spike_counter=0;
    for (int t2=0; t2<=t; t2++)
    {
        if (x[t2]==1)
        {
            spike_counter=spike_counter+(long double)exp((long double)(t2-t));
        }
    }
    y=(long double)exp((-1/tao)*t)+(long double)(tao)*spike_counter; //Synaptics
    return y;
}

//  -------------------------------------------------    MAIN    -----------------------------------------------------------
int main(void)
{
    srand(time(0));
    
//  ----------------------------------------------  Creating Network  ------------------------------------------------------
//  ----------------------------------------------  Sensory Networks  ------------------------------------------------------
    const int Nsensory=512; // Number of neurons in each sensory subnetworks
    
    long double** sensory_wights[8];
    for(int i=0; i<8;i++)
    {
        sensory_wights[i]=new long double*[Nsensory];
        for(int j=0;j<Nsensory;j++)
        {
            sensory_wights[i][j]=new long double[Nsensory];
        }
    }
    
    const long double landa=0.28;
    const long double A=2;
    const long double k1=1 ,k2=0.25;
    
    for (int n=0;n<8;n++)
    {
        for(int i=0; i<512;i++)
        {
            for(int j=0; j<512;j++)
            {
                if (i==j) sensory_wights[n][i][j]=0;
                else
                {
                long double teta=(long double)2*M_PI*(long double)(i-j)/(long double)Nsensory;
                sensory_wights[n][i][j]= landa+A*((long double)exp(k1*((long double)cos(teta)-1))-(long double)exp(k2*((long double)cos(teta)-1)));
                }
            }
        }
    }
    
//  ------------------------------------------- Sensory to Random Network --------------------------------------------------
    const int Nrandom=1024; // Number of Neurons in random network
    
    long double** sensory_random_ffwight[8];
    long double** sensory_random_fbwight[8];
    for(int i=0; i<8;i++)
    {
        sensory_random_fbwight[i]=new long double*[Nsensory];
        sensory_random_ffwight[i]=new long double*[Nrandom];
        for(int j=0;j<Nrandom;j++)
        {
            sensory_random_ffwight[i][j]=new long double[Nsensory];
        }
        for(int j=0;j<Nsensory;j++)
        {
            sensory_random_fbwight[i][j]=new long double[Nrandom];
        }
    }
    
    const long double gama=0.35; // Likelyhood of sensory to random network excitory and inhibitory together
    for (int k=0;k<8;k++)
    {
        for(int i=0;i<Nrandom;i++)
        {
            for(int j=0;j<Nsensory;j++)
            {
                long double temp=(long double)rand()/RAND_MAX;
                if(temp<=gama)
                {
                    sensory_random_ffwight[k][i][j]=1; // excitory conections existance
                    sensory_random_fbwight[k][j][i]=1; // feedforward matrix ~ feedback matrix transpose
                }
                else
                {
                    sensory_random_ffwight[k][i][j]=-1; // inhibitory conections existance
                    sensory_random_fbwight[k][j][i]=-1; // feedforward matrix ~ feedback matrix transpose
                }
            }
        }
    }
    
// --------------------------------------  Wights insialization FF & FB network -------------------------------------------
    long double n_exc_i[Nrandom]; // Number of excitory conections in FF for each neuron i in random network
    for(int i=0;i<Nrandom;i++)
    {
        n_exc_i[i]=0;
        for (int k=0;k<8;k++)
        {
            for (int j=0; j<Nsensory; j++)
            {
                if (sensory_random_ffwight[k][i][j]==1)
                {
                    n_exc_i[i]++;
                }
            }
        }
    }
    
    long double n_exc_j[8][Nsensory]; // Number of excitory conection in FB for each neuron j in sensory network
    for (int k=0; k<8; k++)
    {
        for (int j=0; j<Nsensory; j++)
        {
            n_exc_j[k][j]=0;
            for (int i=0; i<Nrandom; i++)
            {
                if (sensory_random_fbwight[k][j][i]==1)
                {
                    n_exc_j[k][j]++;
                }
            }
        }
    }
    
    const long double alpha=2100;
    const long double beta=200;
       for (int i=0; i<Nrandom; i++)
       {
           for (int k=0; k<8; k++)
           {
               for (int j=0; j<Nsensory; j++)
               {
                   if(sensory_random_ffwight[k][i][j]==1){
                       sensory_random_ffwight[k][i][j]=alpha*((1/(long double)n_exc_i[i])-(1/(long double)(8*Nsensory))); // FF excitory conection
                       sensory_random_fbwight[k][j][i]=beta*((1/(long double)n_exc_j[k][j])-(1/(long double)(Nrandom)));  // FB excitory conection
                   }
                   else{
                       sensory_random_ffwight[k][i][j]=-alpha*(1/(long double)(8*Nsensory)); // FF inhibitory conection
                       sensory_random_fbwight[k][j][i]=-beta*(1/(long double)(Nrandom)); // FB inhibitory conection
                   }
                       
               }
           }
       }
       
//  --------------------------------------------  Balancing FF & FB  Test  ------------------------------------------------
  /*
    for (int i=0; i<Nrandom; i++)
    {
        long double d=0;
        for (int k=0; k<8; k++)
        {
            for (int j=0; j<Nsensory; j++)
            {
     //         cout<<sensory_random_ffwight[k][i][j]<<"\n";
                d=d+sensory_random_ffwight[k][i][j];
            }
     //       cout<<d<<"\n";
        }
    }
    
    
    for (int k=0; k<8; k++)
    {
        for (int i=0; i<Nsensory; i++)
        {
            double d=0;
            for (int j=0; j<Nrandom; j++)
            {
     //         cout<<sensory_random_fbwight[k][i][j]<<"\n";
                d=d+sensory_random_fbwight[k][i][j];
            }
      //    cout<<d<<"\n";
        }
    }
 */
   
//  -----------------------------------------------  Random Network  -----------------------------------------------------
    const int Nstimuli=Nsensory;
    
    long double** stimuli_random_wight[8];
    for (int k=0; k<8; k++)
    {
        stimuli_random_wight[k]=new long double*[Nrandom];
        for (int i=0; i<Nrandom; i++)
        {
            stimuli_random_wight[k][i]=new long double[Nstimuli];
        }
    }
    
//  -----------------------------------------  Stimuli to Random conections ----------------------------------------------
    const long double gama_ext=0.2;
    for (int k=0; k<8; k++)
    {
        for (int j=0; j<Nstimuli; j++)
        {
            for (int i=0; i<Nrandom; i++)
            {
                long double temp=(long double)rand()/RAND_MAX;
                if (temp<=gama_ext) stimuli_random_wight[k][i][j]=1;
                else stimuli_random_wight[k][i][j]=0;
            }
        }
    }
    
    int n_exc_i_ext[Nrandom]; // Number of excitory ext conections for each neuron in random network
    for (int i=0; i<Nrandom; i++)
    {
        for (int k=0; k<8; k++)
        {
            for (int j=0 ; j<Nstimuli; j++)
            {
                if (stimuli_random_wight[k][i][j]) n_exc_i_ext[i]++;
            }
        }
    }
    
    const long double alpha_ext=100;
    for (int i=0; i<Nstimuli; i++)
    {
        for (int k=0; k<8; k++)
        {
            for (int j=0; j<Nstimuli; j++)
            {
                if (stimuli_random_wight[k][i][j])
                {
                    stimuli_random_wight[k][i][j]=alpha_ext*((1/(long double)n_exc_i_ext[i])-(1/((long double)8*Nstimuli)));
                }
            }
        }
    }
    
//  -----------------------------------------  Balancing Stimuli to Random -----------------------------------------------
    for (int k=0; k<8; k++)
    {
        for (int j=0; j<Nstimuli; j++)
        {
            long double diff=0; // difference from balance (0)
            for (int i=0; i<Nrandom; i++)
            {
                diff=diff+stimuli_random_wight[k][i][j];
            }
            while (abs(diff)>abs(diff-(alpha_ext/((long double)8*Nstimuli))))
            {
                int i= rand()%Nrandom;
                diff=diff-(alpha_ext/((long double)8*Nstimuli));
                stimuli_random_wight[k][i][j]=stimuli_random_wight[k][i][j]-(alpha_ext)/((long double)8*Nsensory);
            }
//          cout<<diff<<"\n"; // testing be balance
        }
    }
    
//  -----------------------------------  Recurence Random Networks Conections   --------------------------------------
    long double** random_network_wights;
    random_network_wights =new long double* [Nrandom];
    for (int i=0; i<Nrandom; i++)
    {
        random_network_wights[i]=new long double [Nrandom];
    }
    
    const long double gama_rec=0.2;
    for (int i=0; i<Nrandom; i++)
    {
        for (int j=0; j<Nrandom; j++)
        {
            long double temp=(long double)rand()/RAND_MAX;
            if (temp<gama_rec) random_network_wights[i][j]=1;
            else random_network_wights[i][j]=0;
        }
    }
    
    int n_exc_rand[Nrandom]; //number of excitory for neuron i in random net
    for (int i=0; i<Nrandom; i++)
    {
        n_exc_rand[i]=0;
        for (int j=0; j<Nrandom; j++)
        {
            if(random_network_wights[i][j]) n_exc_rand[i]++;
        }
    }
    
    const long double alpha_rec=250;
    for (int i=0; i<Nrandom; i++)
    {
        for (int j=0; j<Nrandom; j++)
        {
            if (random_network_wights[i][j])
            {
                random_network_wights[i][j]=alpha_rec*((1/(long double)n_exc_rand[i])-(1/(long double)Nrandom));
            }
        }
    }
  
//  -----------------------------------------   Balancing Recurence Network -----------------------------------------------
    for (int i=0; i<Nrandom; i++)
    {
        long double diff=0;
        for (int j=0; j<Nrandom; j++)
        {
            diff=diff+random_network_wights[i][j];
        }
        while(abs(diff)>abs(diff-((long double)alpha_rec/(long double)Nrandom)))
        {
            int j=rand()%Nrandom;
            diff=diff-((long double)alpha_rec/(long double)Nrandom);
            random_network_wights[i][j]=random_network_wights[i][j]-(long double)(alpha_rec/(long double)Nrandom);
        }
//      cout<<diff<<" \n"; // testing be balance
    }

//  ------------------------------------------------    Simulation  ------------------------------------------------------
    const int rtime=1000; // Running time 1000ms = 1s
    const int stime=100; // Stimuli time 100ms = 0.1s
       
    long double** r_sens[8]; // Sensory part
    long double** spike_sens[8];
    long double** s_sens[8];
    for (int k=0; k<8; k++)
    {
        r_sens[k]=new long double* [Nsensory];
        spike_sens[k]=new long double* [Nsensory];
        s_sens[k]=new long double* [Nsensory];
        for (int i=0; i<Nsensory; i++)
        {
            r_sens[k][i]=new long double [rtime];
            spike_sens[k][i]=new long double [rtime];
            s_sens[k][i]=new long double [rtime];
            for (int t=0; t<rtime; t++)
            {
                r_sens[k][i][t]=0;
                spike_sens[k][i][t]=0;
                s_sens[k][i][t]=0;
            }
        }
    }
    
    long double** r_random=new long double* [Nrandom]; //Random part
    long double** spike_random=new long double* [Nrandom];
    long double** s_random=new long double* [Nrandom];
    for (int i=0; i<Nrandom; i++)
    {
        r_random[i]=new long double [rtime];
        spike_random[i]=new long double [rtime];
        s_random[i]=new long double [rtime];
        for (int t=0; t<rtime; t++)
        {
            r_random[i][t]=0;
            spike_random[i][t]=0;
            s_random[i][t]=0;
        }
    }
    
//  ---------------------------------------------   External Stimuli    --------------------------------------------------
    long double** s_ext[8];
    for (int k=0; k<8; k++)
    {
        s_ext[k]=new long double* [Nstimuli];
    }
    for (int k=0; k<8; k++)
    {
        for (int i=0; i<Nstimuli; i++)
        {
            s_ext[k][i]=new long double [rtime];
            for (int t=0; t<rtime; t++)
            {
                s_ext[k][i][t]=0;
            }
        }
    }
    
    const int Nstimuli_nets=8; // Number of subsensory network have input ...
    const long double miu=Nstimuli/2; // in paper random but no deference ...
    const long double sig=Nsensory/32;
    const long double S_EXT=10;
    
    for (int t=0; t<stime; t++)
    {
        for (int k=0; k<Nstimuli_nets; k++)
        {
            for (int i=0; i<Nstimuli; i++)
            {
                if (abs(i-miu)<(3*sig)){
                    s_ext[k][i][t]=(long double)S_EXT*(1/sqrt(2*M_PI*pow(sig,2)))*exp(-pow((long double)abs(i-miu),2)/(2*(long double)pow(sig,2)));
                }
                else {
                    s_ext[k][i][t]=0;
                }
            }
        }
        for (int k=Nstimuli_nets; k<8; k++)
        {
            for (int i=0; i<Nstimuli; i++)
            {
                s_ext[k][i][t]=0;
            }
        }
    }
    
//  -------------------------------------------------   Runinng ---------------------------------------------------------
    
    for (int t=1; t<rtime; t++)
    {
        
        for (int i=0; i<Nrandom; i++) // random part
        {
            long double temp=0;
            for (int k=0; k<8; k++)
            {
                for (int j=0; j<Nsensory; j++)
                {
                    temp=temp+sensory_random_ffwight[k][i][j]*s_sens[k][j][t-1];
                }
            }
            for (int j=0; j<Nrandom; j++)
            {
                temp=temp+random_network_wights[i][j]*s_random[j][t-1];
            }
            for (int k=0; k<8;k++)
            {
                for (int j=0; j<Nstimuli; j++)
                {
                    temp=temp+stimuli_random_wight[k][i][j]*s_ext[k][j][t-1];
                }
            }
            r_random[i][t-1]=fire_rate_function(temp); // Firerate
            spike_random[i][t]=spiking_function(r_random[i][t-1]); // Spike
            s_random[i][t]=synaps_function(t,spike_random[i]); //Synaptics
        }
        
        for (int k=0; k<8; k++) //sensory part
        {
            for (int i=0; i<Nsensory; i++)
            {
                long double temp=0;
                for (int j=0;j<Nrandom; j++)
                    {
                        temp=temp+sensory_random_fbwight[k][i][j]*s_random[j][t-1];
                    }
                for (int j=0; j<Nsensory; j++)
                    {
                        temp=temp+sensory_wights[k][i][j]*s_sens[k][j][t-1];
                    }
                temp=temp+s_ext[k][i][t-1];
                r_sens[k][i][t-1]=fire_rate_function(temp); // Firerate
                spike_sens[k][i][t]=spiking_function(r_sens[k][i][t-1]); // Spike
                s_sens[k][i][t]=synaps_function(t,spike_sens[k][i]); //Synaptics
            }
        }

    }
    
//  ---------------------------------------   Raster Plot of a Trial of Load  --------------------------------------------
    
    for (int k=0; k<8; k++) // sensory networks
    {
        for (int i=0; i<Nsensory; i=i+8)
        {
            for (int t=1; t<rtime; t=t+5)
            {
                if (spike_sens[k][i][t]==0) cout<<" ";
                else if(t-2<stime) cout<<"*";
                else cout<<".";
            }
            cout<<"\n"<<i+8;
        }
    cout<<"\n ----------------------------------------------------------------------------------------------------------------------------------------------------------- \n";
    }
    
    cout<<"\n ########################################################################################################################################################### \n";
    
    for (int i=0; i<Nrandom; i=i+8) // random network
    {
        for (int t=0; t<rtime; t=t+5)
        {
            if (spike_random[i][t]==0) cout<<" ";
            else cout<<".";
        }
        cout<<"\n";
    }
    
//  -------------------------------------------------   TESTING  ----------------------------------------------------------
    
//  -------------------------------------------------   THE END   ---------------------------------------------------------
//  ----------------------------------------------  Memory free up  -------------------------------------------------------
    
    for(int i=0; i<8;i++) // Sensory wights
       {
           for(int j=0;j<Nsensory;j++)
           {
               delete[] sensory_wights[i][j];
           }
           delete[] sensory_wights[i];
       }
    
    for(int i=0; i<8;i++) // Sensory to Random FF & FB
    {
        for(int j=0;j<Nrandom;j++)
        {
            delete[] sensory_random_ffwight[i][j];
        }
        for(int j=0;j<Nsensory;j++)
        {
            delete[] sensory_random_fbwight[i][j];
        }
        
        delete[] sensory_random_fbwight[i];
        delete[] sensory_random_ffwight[i];
    }
    
    for(int k=0; k<8;k++) // Stimuli to Random
          {
              for(int j=0;j<Nstimuli;j++)
              {
                  delete[] stimuli_random_wight[k][j];
              }
              delete[] stimuli_random_wight[k];
          }
       
    for (int i=0; i<Nrandom; i++) // Random wights
    {
        delete [] random_network_wights[i];
    }
    delete[] random_network_wights;
    
    for (int k=0; k<8; k++)// Sensory simulation
    {
        for (int i=0; i<Nsensory; i++)
        {
            delete [] r_sens[k][i];
            delete [] spike_sens[k][i];
            delete [] s_sens[k][i];
        }
        delete [] r_sens[k];
        delete [] spike_sens[k];
        delete [] s_sens[k];
    }
    
    for (int i=0; i<Nrandom; i++) // Random simulation
    {
        delete [] r_random[i];
        delete [] spike_random[i];
        delete [] s_random[i];
    }
    delete [] r_random;
    delete [] spike_random;
    delete [] s_random;

    for (int k=0; k<8; k++) // External simulation
    {
        for (int i=0; i<Nstimuli; i++)
        {
            delete [] s_ext[k][i];
        }
        delete [] s_ext[k];
    }
    
    return 0;
}
