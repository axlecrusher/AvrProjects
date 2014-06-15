
/*
	This is a program to generate a step count lookup table.
	g++ -Wall  main.cpp
*/

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <cmath>

#include <vector>

using namespace std;

#define TICK_SEC 10

const float BOARD_LENGTH = 18.125f; //inches
const float SIDEREAL_DAY = 86164.1f; //seconds
const double RAD_SEC = 0.00007292115053925691183364402072973553682327487665; //radians per second
const double RAD_TIME_TICK = 0.00000729211505392569118336440207297355368232748766;

const uint32_t ROD_THREADS = 20; //threads per inch
const uint32_t STEPS = 200; //stepper motor steps per 360 degrees
const uint32_t STEPS_INCH = ROD_THREADS*STEPS; //stepper motor steps per 360 degrees

const uint32_t ENDTIME = 10800*2; //6 hours

struct StepData
{
	uint32_t seconds;
	double radians;
	double screw_length;
	uint32_t steps;
	double arc_seconds;
};

double ComputeScrewLength(double r)
{
	return sqrt((BOARD_LENGTH*BOARD_LENGTH) + (BOARD_LENGTH*BOARD_LENGTH) - (2*BOARD_LENGTH*BOARD_LENGTH*cos(r)));
}

double ComputeSteps(double length)
{
	return length*STEPS_INCH;
}

StepData ComputeStepData(uint32_t seconds)
{
	StepData d;
	d.seconds = seconds;
	d.radians = RAD_SEC*seconds;
	d.arc_seconds = d.radians*206264.806;
	d.screw_length = ComputeScrewLength(d.radians);
	d.steps = round(ComputeSteps(d.screw_length));
	return d;
}

float ArcSecondsFromSteps(uint32_t steps)
{
	double length = steps/STEPS_INCH; //only use whole number steps
	double theta = acos(((BOARD_LENGTH*BOARD_LENGTH) + (BOARD_LENGTH*BOARD_LENGTH) - (length*length))/(2*BOARD_LENGTH*BOARD_LENGTH));
	return theta*206264.806f;
}

float ArcSecondsFromSteps(double steps)
{
	double length = steps/STEPS_INCH; //only use whole number steps
	double theta = acos(((BOARD_LENGTH*BOARD_LENGTH) + (BOARD_LENGTH*BOARD_LENGTH) - (length*length))/(2*BOARD_LENGTH*BOARD_LENGTH));
	return theta*206264.806f;
}

float ComputeArcSecondError(const StepData& d)
{
	float arc_sec = ArcSecondsFromSteps(d.steps);
	return d.arc_seconds-arc_sec;
}

void PrintStepData(const StepData& d)
{
	printf("%d %d %.10f\n", d.seconds, d.steps, d.radians);
}

float lerp(uint32_t time0, uint32_t time1, uint32_t step0, uint32_t step1, uint32_t time_x)
{
	float a = float(time_x-time0);
	a/=(time1-time0);
	return step0+(step1-step0)*a;
}

vector<StepData> datas;

float ComputeMaxArcSecError(const StepData& n,const StepData& o)
{
	float half_time_sec = (n.seconds-o.seconds)*0.5 + o.seconds; //half way
	float arcSec = RAD_SEC*half_time_sec*206264.806f;

	float steps_p = lerp(o.seconds*TICK_SEC, n.seconds*TICK_SEC, o.steps, n.steps, half_time_sec*TICK_SEC);
	float arcSec_p = ArcSecondsFromSteps(steps_p);

//	printf("%d %f %d %f %f\n", o.seconds, half_time_sec, n.seconds, steps, 0.0);

	return arcSec-arcSec_p;
}


void BuildLookupTable()
{
	StepData prev = ComputeStepData(0);
	datas.push_back(prev);

	for (uint32_t s = 1; s<=ENDTIME; ++s)
	{
		StepData ns = ComputeStepData(s);
		float error = ComputeMaxArcSecError(ns,prev);
//		printf("%d %f\n",s, error);

		//midpoint is probably the most inaccurate spot, unless we intersect cos(x) somewhere
		if (abs(error)>=2.0f)
		{
			prev = ComputeStepData(s-1);
			datas.push_back(prev);
		}
	}

	datas.push_back(ComputeStepData(ENDTIME));
}

void print_table()
{
	for (uint32_t i = 0; i<datas.size();++i)
	{
		printf("%d %d\n",datas[i].seconds, datas[i].steps);
	}	
}

void print_table_hex_array()
{
	for (uint32_t i = 0; i<datas.size();++i)
	{
		if ((i%10)==0 && i>0) printf("\n");
//		uint32_t steps = round(datas[i].steps);
		printf("0x%05X,0x%05X,",datas[i].seconds, datas[i].steps);
	}	
}

/* compress by subtracting previous table values */
void print_compressed()
{
	uint32_t last_time = 0;
	uint32_t last_step = 0;

	for (uint32_t i = 0; i<datas.size();++i)
	{
		uint32_t t = datas[i].seconds;
		uint32_t s = datas[i].steps;

		printf("%d %d\n",t-last_time, s-last_step);

		last_time=t;
		last_step = s;
//		PrintStepData(datas[i]);
	}	
}

void print_compressed_hex_array()
{
	uint32_t last_time = 0;
	uint32_t last_step = 0;

	for (uint32_t i = 0; i<datas.size();++i)
	{
		uint32_t t = datas[i].seconds;
		uint32_t s = datas[i].steps;

		if ((i%10)==0 && i>0) printf("\n");
		printf("0x%04X,0x%04X,",t-last_time, s-last_step);

		last_time=t;
		last_step = s;
//		PrintStepData(datas[i]);
	}	
}

int main()
{
	BuildLookupTable();
//	print_table();
//	print_compressed();
	print_compressed_hex_array();
//	print_table_hex_array();
}

/*

Copyright (c) 2014, Joshua Allen
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Joshua Allen nor the names of other contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
