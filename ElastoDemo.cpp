// ElastoDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Windows.h>
#include <mex.h>
#include <mat.h>
#include <math.h>
#include <fstream>
#include <cstdlib>

//data
const char  *pFileIn1 = "data//rfdata1.mat";

const char  *pFileIn2 = "data//rfdata2.mat";
const char  *pFileOut = "data//Out.mat";

//弹性计算相关参数
const double MaxAxisDisp[2] = { -100, 100 };
const double MaxLateralDisp[2] = { -2, 2 };
const double SeedLineNum = 254;
const double DpRegularWeight = 0.15;
const double AxisRegularWeight = 5;
const double LateralRegularWeithtTop = 10;
const double LateralRegularWeithtPre = 0.005;
const double IrlsTher = 0.2;
const double AttCoeff = 1.002032936054392;
const double wDiff = 43;

//using namespace std;

int main()
{	
	typedef   DWORD(WINAPI   *MYFUNC)(int, mxArray**, int, mxArray**);
	HINSTANCE   hDllInst1 = LoadLibrary("bin//CalcDisp.dll");
	HINSTANCE   hDllInst2 = LoadLibrary("bin//CalcStrain.dll");
	if (hDllInst1 == NULL || hDllInst2 == NULL)
	{
		return 1;
	}
	MYFUNC CalcDisp = (MYFUNC)GetProcAddress(hDllInst1, "mexFunction");
    MYFUNC CalcStrain = (MYFUNC)GetProcAddress(hDllInst2, "mexFunction");

	//载入data和参数
	MATFile* pFile1 = matOpen(pFileIn1, "r");
	mxArray* pRfData1 = matGetVariable(pFile1, "Im1");
	matClose(pFile1);
	MATFile* pFile2 = matOpen(pFileIn2, "r");
	mxArray* pRfData2 = matGetVariable(pFile2, "Im2");
	matClose(pFile2);


	//***************************1.计算位移场****************************
	//输入参数
	mxArray* prhs[11];
	prhs[0] = pRfData1;
	prhs[1] = pRfData2;
	prhs[2] = mxCreateDoubleMatrix(1, 2, mxREAL);
	mxSetPr(prhs[2], (double*)MaxAxisDisp);
	prhs[3] = mxCreateDoubleMatrix(1, 2, mxREAL);
	mxSetPr(prhs[3], (double*)MaxLateralDisp);
	prhs[4] = mxCreateDoubleMatrix(1, 1, mxREAL);
	mxSetPr(prhs[4], (double*)&SeedLineNum);
	prhs[5] = mxCreateDoubleMatrix(1, 1, mxREAL);
	mxSetPr(prhs[5], (double*)&DpRegularWeight);
	prhs[6] = mxCreateDoubleMatrix(1, 1, mxREAL);
	mxSetPr(prhs[6], (double*)&AxisRegularWeight);
	prhs[7] = mxCreateDoubleMatrix(1, 1, mxREAL);
	mxSetPr(prhs[7], (double*)&LateralRegularWeithtTop);
	prhs[8] = mxCreateDoubleMatrix(1, 1, mxREAL);
	mxSetPr(prhs[8], (double*)&LateralRegularWeithtPre);
	prhs[9] = mxCreateDoubleMatrix(1, 1, mxREAL);
	mxSetPr(prhs[9], (double*)&IrlsTher);
	prhs[10] = mxCreateDoubleMatrix(1, 1, mxREAL);
	mxSetPr(prhs[10], (double*)&AttCoeff);

	//输出参数	
	mxArray* plhs[3];
	size_t RowSize = mxGetM(pRfData1);
	size_t ColSize = mxGetN(pRfData1);
	plhs[0] = mxCreateDoubleMatrix(RowSize, ColSize, mxREAL);
	plhs[1] = mxCreateDoubleMatrix(RowSize, ColSize, mxREAL);
	plhs[2] = mxCreateDoubleMatrix(RowSize, 2, mxREAL);

    //调用DLL内函数
	CalcDisp(3, plhs, 11, prhs);

	//输出位移场
	mxArray* pD1 = plhs[0];
	mxArray* pD2 = plhs[1];
	mxArray* pDpDisparity = plhs[2];

	//***************************2.计算应变场**********************
	//输入参数
	mxArray* prhsStrain[2];
	prhsStrain[0] = pD1;
	prhsStrain[1] = mxCreateDoubleMatrix(RowSize, RowSize, mxREAL);
	mxSetPr(prhsStrain[1], (double*)&wDiff);

	//输出参数
	mxArray* plhsStrain[2];
	plhsStrain[0] = mxCreateDoubleMatrix(RowSize, RowSize, mxREAL);
	plhsStrain[1] = mxCreateDoubleMatrix(RowSize, RowSize, mxREAL);

	// 调用DLL内函数
	CalcStrain(2, plhsStrain, 2, prhsStrain);

	//输出应变场
	mxArray* pStrain1 = plhsStrain[0];
	mxArray* pStrain2 = plhsStrain[1];

	//写输出文件
	MATFile* pFile = matOpen(pFileOut, "w");
	matPutVariable(pFile, "strain1", pStrain1);

	FreeLibrary(hDllInst1);
	FreeLibrary(hDllInst2);
    return 0;
}

