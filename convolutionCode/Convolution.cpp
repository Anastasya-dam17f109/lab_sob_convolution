// Convolution.cpp: ���������� ����� ����� ��� ����������� ����������.
//

#include "stdafx.h"
#include <conio.h>
#include <stdint.h>
#include <omp.h>
#include <iostream>

typedef union
{
	void* m_pv;
	int64_t     m_i64;
} unionPtrToInt64;

//================================================================
// class MFData
//================================================================
class MFData
{
public:
	MFData();
	~MFData();
	void deleteData();

	int iRead(const char* a_pcFileName);
	int iCreate(int64_t a_i64W, int64_t a_i64H, int64_t a_i64Pow);
	int iWrite(const char* a_pcFileName);

	float* pfGetRow(int64_t a_i64Row) { return m_ppRows[a_i64Row]; }

public:
	int64_t	m_i64W;			// ������
	int64_t	m_i64H;			// ������
	int64_t	m_i64Pow;			// ������������ (������� 2)
	int64_t	m_i64LenSize;		// ����� ������ � ���������, � ������ ������������ (m_i64LenSize >= m_i64W)

	int8_t* m_pData;			// �� ����������� ���������� ��� ������ ������
	float** m_ppRows;		// ������ ���������� �����, ��������� �� ������ ������ - ������ ����������� ������, ��� ������ ���� ���� �� ������, ���������� ��������� � ������ - m_i64LenSize
};

MFData::MFData()
	: m_i64W(0),
	m_i64H(0),
	m_i64Pow(0),
	m_i64LenSize(0),
	m_pData(nullptr),
	m_ppRows(nullptr)
{
}

MFData::~MFData()
{
	deleteData();
}

void MFData::deleteData()
{
	if (nullptr != m_ppRows)
	{
		delete[] m_ppRows;
		m_ppRows = nullptr;
	}

	if (nullptr != m_pData)
	{
		delete[] m_pData;
		m_pData = nullptr;
	}
	m_i64W = 0;
	m_i64H = 0;
	m_i64Pow = 0;
	m_i64LenSize = 0;
}

int MFData::iRead(const char* a_pcFileName)
{
	deleteData();

	FILE* pf;
	int iRet = fopen_s(&pf, a_pcFileName, "rb");
	if (0 != iRet)
		return iRet;

	while (true)
	{
		if (fread(&m_i64W, sizeof(int64_t), 4, pf) != 4)
		{
			iRet = -2;
			break;
		}

		// ������� ������
		int64_t i64Align = static_cast<int64_t>(1) << m_i64Pow, i64;
		int64_t i64CountEl = m_i64LenSize * m_i64H;
		if (i64Align > 4)
			i64CountEl += ((i64Align << 1) >> 2);
		else
			i64CountEl++;
		m_pData = new int8_t[i64CountEl * sizeof(float)];
		m_ppRows = new float* [m_i64H];

		// ����������� � ��������� ������ ��������
		unionPtrToInt64 ui;
		ui.m_pv = m_pData;
		ui.m_i64 += (i64Align - (ui.m_i64 & (i64Align - 1))) & (i64Align - 1);
		m_ppRows[0] = static_cast<float*>(ui.m_pv);
		for (i64 = 1; i64 < m_i64H; i64++)
			m_ppRows[i64] = m_ppRows[i64 - 1] + m_i64LenSize;

		// ������ ���������� ������
		if (fread(pfGetRow(0), sizeof(float), m_i64LenSize * m_i64H, pf) != (m_i64LenSize * m_i64H))
		{
			iRet = -3;
			break;
		}
		break;
	}
	fclose(pf);
	return iRet;
}

int MFData::iCreate(int64_t a_i64W, int64_t a_i64H, int64_t a_i64Pow)
{
	deleteData();

	m_i64W = a_i64W;
	m_i64H = a_i64H;
	m_i64Pow = a_i64Pow;

	int64_t i64Align = static_cast<int64_t>(1) << m_i64Pow, i64;
	m_i64LenSize = (m_i64W << 2);					// ��������� � �����
	m_i64LenSize += (i64Align - (m_i64LenSize & (i64Align - 1))) & (i64Align - 1);
	m_i64LenSize >>= 2;							// ��������� � ��������

	// ������� ������
	int64_t i64CountEl = m_i64LenSize * m_i64H;
	if (i64Align > 4)
		i64CountEl += ((i64Align << 1) >> 2);
	else
		i64CountEl++;
	m_pData = new int8_t[i64CountEl * sizeof(float)];
	m_ppRows = new float* [m_i64H];

	// ����������� � ��������� ������ ��������
	unionPtrToInt64 ui;
	ui.m_pv = m_pData;
	ui.m_i64 += (i64Align - (ui.m_i64 & (i64Align - 1))) & (i64Align - 1);
	m_ppRows[0] = static_cast<float*>(ui.m_pv);
	for (i64 = 1; i64 < m_i64H; i64++)
		m_ppRows[i64] = m_ppRows[i64 - 1] + m_i64LenSize;

	return 0;
}

int MFData::iWrite(const char* a_pcFileName)
{
	FILE* pf;
	int iRet = fopen_s(&pf, a_pcFileName, "wb");
	if (0 != iRet)
		return iRet;

	while (true)
	{
		if (fwrite(&m_i64W, sizeof(int64_t), 4, pf) != 4)
		{
			iRet = -2;
			break;
		}

		// ����� ���������� ������
		if (fwrite(pfGetRow(0), sizeof(float), m_i64LenSize * m_i64H, pf) != (m_i64LenSize * m_i64H))
		{
			iRet = -3;
			break;
		}
		break;
	}
	fclose(pf);
	return iRet;
}

//================================================================
//================================================================
// ������� ��������� �����������
//������ ����� ������ ������� - ��� ��������� ����������� �� ���������������� � ������������������ �������
//�������� ����������� ��-�� ������ ���������� ���  ���� float, ������� ��������� ��-�� ������  �������� ������������
// (�� ��-������� ���������� � ������, ������ � ����� �� ���)
void comparison(MFData& res1, MFData& res2)
{
	int counter = 0;
	int64_t iRow, iCol;
	float* res1Row, * res2Row;
	for (iRow = 0; iRow < res1.m_i64H; iRow++)
	{
		res1Row = res1.pfGetRow(iRow);
		res2Row = res2.pfGetRow(iRow);
		for (iCol = 0; iCol < res1.m_i64W; iCol++)
		{
			if (res1Row[iCol] != res2Row[iCol])
			{
				counter++;
				std::cout << res1Row[iCol] << " " << res2Row[iCol] << std::endl;
				std::cout << iRow << " " << iCol << std::endl;
			}
		}
	}
	if (counter > 0)
		std::cout << "results are not ecual" << std::endl;
	else
		std::cout << "results are  ecual" << std::endl;
}

//

int iConvolution(MFData& ar_cmmIn, MFData& ar_cmmWin, MFData& ar_cmmOut)
{
	int64_t iWinX = ar_cmmWin.m_i64W;
	int64_t iWinY = ar_cmmWin.m_i64H;
	if ((1 != (iWinX & 1)) || (iWinX < 3) || (1 != (iWinY & 1)) || (iWinY < 3))
		return 2;	// ���� �.�. ��������� ������� � >=3
	int64_t iWinX_2 = iWinX >> 1;
	int64_t iWinY_2 = iWinY >> 1;

	ar_cmmOut.deleteData();
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, ar_cmmIn.m_i64Pow);

	int64_t iRow, iCol, iX, iY, iR, iC;
	float* pfRowOut, * pfRowIn, * pfRowWin;
	for (iRow = 0; iRow < ar_cmmIn.m_i64H; iRow++)
	{
		pfRowOut = ar_cmmOut.pfGetRow(iRow);
		for (iCol = 0; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			float f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR < 0)
					pfRowIn = ar_cmmIn.pfGetRow(-iR);
				else if (iR >= ar_cmmIn.m_i64H)
					pfRowIn = ar_cmmIn.pfGetRow(2 * ar_cmmIn.m_i64H - iR - 2);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				for (iX = -iWinX_2; iX <= iWinX_2; iX++)
				{
					iC = iCol + iX;
					if (iC < 0)
						iC = -iC;
					else if (iC >= ar_cmmIn.m_i64W)
						iC = 2 * ar_cmmIn.m_i64W - iC - 2;

					f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
				}
			}
			pfRowOut[iCol] = f;
		}
	}
	return 0;
}

// ����������� ������� ����� �� ����������
// �������� ����� �����������: ����� �������� ��� � ��������� ���������� if ������
// ������ ��������� �� ��������
// ���������� ����� 3 ������� ����� �� ������� ������� � � ������
// 3 ��������� �� ��������

int iConvolution_v2(MFData& ar_cmmIn, MFData& ar_cmmWin, MFData& ar_cmmOut)
{
	int64_t iWinX = ar_cmmWin.m_i64W;
	int64_t iWinY = ar_cmmWin.m_i64H;
	if ((1 != (iWinX & 1)) || (iWinX < 3) || (1 != (iWinY & 1)) || (iWinY < 3))
		return 2;	// ���� �.�. ��������� ������� � >=3
	int64_t iWinX_2 = iWinX >> 1;
	int64_t iWinY_2 = iWinY >> 1;

	ar_cmmOut.deleteData();
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, ar_cmmIn.m_i64Pow);
	float f;
	int64_t iRow, iCol, iX, iY, iR, iC;
	float* pfRowOut, * pfRowIn, * pfRowWin;
	// ������������ ������� ������� ����������� (���� �� ���������), ��� ������� ������ ��������� �� ���������
	for (iRow = 0; iRow < iWinY_2; iRow++)
	{
		pfRowOut = ar_cmmOut.pfGetRow(iRow);
		// ������������ �� ����� �����, ������ ��������� ������ �� ����������� (�� ��������)
		for (iCol = 0; iCol < iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				// ��������������� ���������
				if (iR < 0)
					pfRowIn = ar_cmmIn.pfGetRow(-iR);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				for (iX = -iWinX_2; iX <= iWinX_2; iX++)
				{
					iC = iCol + iX;
					if (iC < 0)
						iC = -iC;

					f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
				}
			}
			pfRowOut[iCol] = f;
		}
		// ������������ ����� ������� ������� (������ �� ����������� �� ��������)
		for (iCol = iWinX_2; iCol < ar_cmmIn.m_i64W - iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR < 0)
					pfRowIn = ar_cmmIn.pfGetRow(-iR);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				for (iX = -iWinX_2; iX <= iWinX_2; iX++)
				{
					iC = iCol + iX;
					f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
				}
			}
			pfRowOut[iCol] = f;
		}
		// ������������ ������ ����� ������� �������, ������ ��������� ������ �� ����������� (�� ��������)
		for (iCol = ar_cmmIn.m_i64W - iWinX_2; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR < 0)
					pfRowIn = ar_cmmIn.pfGetRow(-iR);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				for (iX = -iWinX_2; iX <= iWinX_2; iX++)
				{
					iC = iCol + iX;
					if (iC >= ar_cmmIn.m_i64W)
						iC = 2 * ar_cmmIn.m_i64W - iC - 2;
					f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
				}
			}
			pfRowOut[iCol] = f;
		}
	}

	// ������������ ����������� ����� ����������� (�� ���������) 
	// �� ����������� �������� �� �� ���������, ��� ���� � ���������� �����
	for (iRow = iWinY_2; iRow < ar_cmmIn.m_i64H - iWinY_2; iRow++)
	{
		pfRowOut = ar_cmmOut.pfGetRow(iRow);
		for (iCol = 0; iCol < iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				for (iX = -iWinX_2; iX <= iWinX_2; iX++)
				{
					iC = iCol + iX;
					if (iC < 0)
						iC = -iC;
					f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = iWinX_2; iCol < ar_cmmIn.m_i64W - iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				for (iX = -iWinX_2; iX <= iWinX_2; iX++)
				{
					iC = iCol + iX;

					f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = ar_cmmIn.m_i64W - iWinX_2; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				for (iX = -iWinX_2; iX <= iWinX_2; iX++)
				{
					iC = iCol + iX;
					if (iC >= ar_cmmIn.m_i64W)
						iC = 2 * ar_cmmIn.m_i64W - iC - 2;
					f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
				}
			}
			pfRowOut[iCol] = f;
		}
	}

	// ������������ ������ �������, ������ ��������� ����������� ���� �� ���������
	// �� ����������� ��� ��������� �������� ������
	for (iRow = ar_cmmIn.m_i64H - iWinY_2; iRow < ar_cmmIn.m_i64H; iRow++)
	{
		pfRowOut = ar_cmmOut.pfGetRow(iRow);
		for (iCol = 0; iCol < iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR >= ar_cmmIn.m_i64H)
					pfRowIn = ar_cmmIn.pfGetRow(2 * ar_cmmIn.m_i64H - iR - 2);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				for (iX = -iWinX_2; iX <= iWinX_2; iX++)
				{
					iC = iCol + iX;
					if (iC < 0)
						iC = -iC;

					f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = iWinX_2; iCol < ar_cmmIn.m_i64W - iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR >= ar_cmmIn.m_i64H)
					pfRowIn = ar_cmmIn.pfGetRow(2 * ar_cmmIn.m_i64H - iR - 2);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				for (iX = -iWinX_2; iX <= iWinX_2; iX++)
				{
					iC = iCol + iX;
					f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = ar_cmmIn.m_i64W - iWinX_2; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR >= ar_cmmIn.m_i64H)
					pfRowIn = ar_cmmIn.pfGetRow(2 * ar_cmmIn.m_i64H - iR - 2);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				for (iX = -iWinX_2; iX <= iWinX_2; iX++)
				{
					iC = iCol + iX;
					if (iC >= ar_cmmIn.m_i64W)
						iC = 2 * ar_cmmIn.m_i64W - iC - 2;
					f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
				}
			}
			pfRowOut[iCol] = f;
		}
	}


	return 0;
}

// ������� � ������������ 3*3
// ���� ������� � ���, ����� �������������� ���������� ������� 3*3 � iConvolution_v2
// ����� - ����� ��� �������� ����������� ���� �����
// � ������ �������� ��� ������� �� ����������� ���� ������������ ����� ������ �� 3� ������������
int iConvolution_v3(MFData& ar_cmmIn, MFData& ar_cmmWin, MFData& ar_cmmOut)
{
	int64_t iWinX = ar_cmmWin.m_i64W;
	int64_t iWinY = ar_cmmWin.m_i64H;
	if ((1 != (iWinX & 1)) || (iWinX < 3) || (1 != (iWinY & 1)) || (iWinY < 3))
		return 2;	// ���� �.�. ��������� ������� � >=3
	int64_t iWinX_2 = iWinX >> 1;
	int64_t iWinY_2 = iWinY >> 1;

	ar_cmmOut.deleteData();
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, ar_cmmIn.m_i64Pow);
	float f;
	int64_t iRow, iCol, iX, iY, iR, iC;
	float* pfRowOut, * pfRowIn, * pfRowWin;
	// ����� ���� �����-��, ��� ���� � iConvolution_v2 �� ������ ����� �������������� ���������
	for (iRow = 0; iRow < iWinY_2; iRow++)
	{
		pfRowOut = ar_cmmOut.pfGetRow(iRow);
		for (iCol = 0; iCol < iWinX_2; iCol++)
		{
			f = 0;
			// ���� �� �������� (������ ����) ������� �������
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				//�������  ������������� �������
				iR = iRow + iY;
				if (iR < 0)
					pfRowIn = ar_cmmIn.pfGetRow(-iR);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				// ������ ��������� ������ ������ �����������  ������ 3*3
				if (iWinX_2 > 1) // ���������� �������� ������ ������� � 1
				{
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC < 0)
							iC = -iC;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				}
				else
				{
					//������ 3*3
					// ��� � ������ ��������� ����� ������
					f += pfRowIn[iCol + 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol + 1] * pfRowWin[2];
					//�� ���� ��������� ������ ���� ��� ���������� � ��������� ��  ��������
					// ������� ������� �� ����, ������ �� �� ���������, � � ����� �������
				}

			}
			pfRowOut[iCol] = f;
		}

		for (iCol = iWinX_2; iCol < ar_cmmIn.m_i64W - iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR < 0)
					pfRowIn = ar_cmmIn.pfGetRow(-iR);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1) // ������ 3*3
				{
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol + 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = ar_cmmIn.m_i64W - iWinX_2; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR < 0)
					pfRowIn = ar_cmmIn.pfGetRow(-iR);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1) 
				{
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC >= ar_cmmIn.m_i64W)
							iC = 2 * ar_cmmIn.m_i64W - iC - 2;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];	
				}
			}
			pfRowOut[iCol] = f;
		}
	}


	for (iRow = iWinY_2; iRow < ar_cmmIn.m_i64H - iWinY_2; iRow++)
	{
		pfRowOut = ar_cmmOut.pfGetRow(iRow);
		for (iCol = 0; iCol < iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1) 
				{
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC < 0)
							iC = -iC;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = iWinX_2; iCol < ar_cmmIn.m_i64W - iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1)
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;

						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = ar_cmmIn.m_i64W - iWinX_2; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1)
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC >= ar_cmmIn.m_i64W)
							iC = 2 * ar_cmmIn.m_i64W - iC - 2;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}
	}

	for (iRow = ar_cmmIn.m_i64H - iWinY_2; iRow < ar_cmmIn.m_i64H; iRow++)
	{
		pfRowOut = ar_cmmOut.pfGetRow(iRow);
		for (iCol = 0; iCol < iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR >= ar_cmmIn.m_i64H)
					pfRowIn = ar_cmmIn.pfGetRow(2 * ar_cmmIn.m_i64H - iR - 2);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1)
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC < 0)
							iC = -iC;

						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = iWinX_2; iCol < ar_cmmIn.m_i64W - iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR >= ar_cmmIn.m_i64H)
					pfRowIn = ar_cmmIn.pfGetRow(2 * ar_cmmIn.m_i64H - iR - 2);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1)
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = ar_cmmIn.m_i64W - iWinX_2; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR >= ar_cmmIn.m_i64H)
					pfRowIn = ar_cmmIn.pfGetRow(2 * ar_cmmIn.m_i64H - iR - 2);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1)
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC >= ar_cmmIn.m_i64W)
							iC = 2 * ar_cmmIn.m_i64W - iC - 2;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}
	}


	return 0;
}

// ������ �� ����� ��� ������� �������� iConvolution_v3
// ��� ����� ��������������� �������: ����� ������ �������� � ���� ������� � ��� ������, ������� ����������� ���
// ���������� 3*3. ����� ��� ������������� ����� ��������� ���� ��������� �������������  �������� �� �����,
// ������� ��������  ����� ��������� �� ������� ������� �� �� ������ ������� 3*3
// ������  - ��� ���� ����������� ������ ��� ���������� ����� �����������
//� ��� ������ ��-�� ��������� ����, ��� �� �������� ����������� ������������ ������������ ����,
// ��� ������ �������� � �������
// ������� ����� � ������ ���� �������������� ��� � iConvolution_v3. ��������� �������� ������ 
// ��� ����������� ����� ����������� 
int iConvolution_v4(MFData& ar_cmmIn, MFData& ar_cmmWin, MFData& ar_cmmOut)
{
	int64_t iWinX = ar_cmmWin.m_i64W;
	int64_t iWinY = ar_cmmWin.m_i64H;
	if ((1 != (iWinX & 1)) || (iWinX < 3) || (1 != (iWinY & 1)) || (iWinY < 3))
		return 2;	// ���� �.�. ��������� ������� � >=3
	int64_t iWinX_2 = iWinX >> 1;
	int64_t iWinY_2 = iWinY >> 1;

	ar_cmmOut.deleteData();
	ar_cmmOut.iCreate(ar_cmmIn.m_i64W, ar_cmmIn.m_i64H, ar_cmmIn.m_i64Pow);
	float f;
	int64_t iRow, iCol, iX, iY, iR, iC, iD;
	float* pfRowOut, * pfRowIn, * pfRowWin;
	for (iRow = 0; iRow < iWinY_2; iRow++)
	{
		pfRowOut = ar_cmmOut.pfGetRow(iRow);
		for (iCol = 0; iCol < iWinX_2; iCol++)
		{
			f = 0;
			
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = (iR < 0) ? pfRowIn = ar_cmmIn.pfGetRow(-iR) : pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1) 
				{
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC < 0)
							iC = -iC;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				}
				else
				{
					f += pfRowIn[iCol + 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol + 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}
		// ��������� ����������� ����� �� �����������
		for (iCol = iWinX_2; iCol < ar_cmmIn.m_i64W - iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = (iR < 0) ? pfRowIn = ar_cmmIn.pfGetRow(-iR) : pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				// ��������� ��� 3*3
				f += pfRowIn[iCol - 1] * pfRowWin[iWinX_2 - 1]
					+ pfRowIn[iCol] * pfRowWin[iWinX_2]
					+ pfRowIn[iCol + 1] * pfRowWin[iWinX_2 + 1];
				// ����������� ������� ������ � ����� �� ����������� ��������, ���� ����
				// ��� ���� ����� ����� ������������ ��, ��� ������������ ����� - ������������
				if (iWinX_2 > 1) 
				{
					for (iX =  2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						iD = iCol - iX;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2] + pfRowIn[iD] * pfRowWin[iWinX_2 - iX];
					}
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = ar_cmmIn.m_i64W - iWinX_2; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR < 0)
					pfRowIn = ar_cmmIn.pfGetRow(-iR);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1) // ������ 3*3
				{
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC >= ar_cmmIn.m_i64W)
							iC = 2 * ar_cmmIn.m_i64W - iC - 2;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}
	}

	
	for (iRow = iWinY_2; iRow < ar_cmmIn.m_i64H - iWinY_2; iRow++)
	{
		pfRowOut = ar_cmmOut.pfGetRow(iRow);
		for (iCol = 0; iCol < iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1) // ������ 3*3
				{
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC < 0)
							iC = -iC;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = iWinX_2; iCol < ar_cmmIn.m_i64W - iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				f += pfRowIn[iCol - 1] * pfRowWin[iWinX_2 - 1]
					+ pfRowIn[iCol] * pfRowWin[iWinX_2]
					+ pfRowIn[iCol + 1] * pfRowWin[iWinX_2 + 1];
				if (iWinX_2 > 1) // ������ 3*3
				{
					for (iX = 2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						iD = iCol - iX;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2] + pfRowIn[iD] * pfRowWin[iWinX_2 - iX];
					}
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = ar_cmmIn.m_i64W - iWinX_2; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1)
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC >= ar_cmmIn.m_i64W)
							iC = 2 * ar_cmmIn.m_i64W - iC - 2;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}
	}

	for (iRow = ar_cmmIn.m_i64H - iWinY_2; iRow < ar_cmmIn.m_i64H; iRow++)
	{
		pfRowOut = ar_cmmOut.pfGetRow(iRow);
		for (iCol = 0; iCol < iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				if (iR >= ar_cmmIn.m_i64H)
					pfRowIn = ar_cmmIn.pfGetRow(2 * ar_cmmIn.m_i64H - iR - 2);
				else
					pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1)
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC < 0)
							iC = -iC;

						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}
;
		for (iCol = iWinX_2; iCol < ar_cmmIn.m_i64W - iWinX_2; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = (iR >= ar_cmmIn.m_i64H) ? ar_cmmIn.pfGetRow(2 * ar_cmmIn.m_i64H - iR - 2) : pfRowIn = ar_cmmIn.pfGetRow(iR);
				
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				f += pfRowIn[iCol - 1] * pfRowWin[iWinX_2 - 1]
					+ pfRowIn[iCol] * pfRowWin[iWinX_2]
					+ pfRowIn[iCol + 1] * pfRowWin[iWinX_2 + 1];
				if (iWinX_2 > 1) // ������ 3*3
				{
					for (iX = 2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						iD = iCol - iX;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2] + pfRowIn[iD] * pfRowWin[iWinX_2 - iX];
					}
				}
			}
			pfRowOut[iCol] = f;
		}

		for (iCol = ar_cmmIn.m_i64W - iWinX_2; iCol < ar_cmmIn.m_i64W; iCol++)
		{
			f = 0;
			for (iY = -iWinY_2; iY <= iWinY_2; iY++)
			{
				iR = iRow + iY;
				pfRowIn = (iR >= ar_cmmIn.m_i64H) ? ar_cmmIn.pfGetRow(2 * ar_cmmIn.m_i64H - iR - 2) : pfRowIn = ar_cmmIn.pfGetRow(iR);
				pfRowWin = ar_cmmWin.pfGetRow(iY + iWinY_2);
				if (iWinX_2 > 1)
					for (iX = -iWinX_2; iX <= iWinX_2; iX++)
					{
						iC = iCol + iX;
						if (iC >= ar_cmmIn.m_i64W)
							iC = 2 * ar_cmmIn.m_i64W - iC - 2;
						f += pfRowIn[iC] * pfRowWin[iX + iWinX_2];
					}
				else
				{
					f += pfRowIn[iCol - 1] * pfRowWin[0]
						+ pfRowIn[iCol] * pfRowWin[1]
						+ pfRowIn[iCol - 1] * pfRowWin[2];
				}
			}
			pfRowOut[iCol] = f;
		}
	}


	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	MFData cmmIn, cmmWin, cmmOut, cmmOut1;
	double dStart, dEnd;

	cmmIn.iRead("D:/Convolution/Convolution/in.mfd");

	// 3x3
	cmmWin.iRead("D:/Convolution/Convolution/blur3.mfd");

	dStart = omp_get_wtime();
	iConvolution(cmmIn, cmmWin, cmmOut);
	dEnd = omp_get_wtime();
	printf("3x3 - %lg sec.\n", dEnd - dStart);

	dStart = omp_get_wtime();
	iConvolution_v3(cmmIn, cmmWin, cmmOut1);
	dEnd = omp_get_wtime();
	printf("3x3 - %lg sec.\n", dEnd - dStart);
	//comparison(cmmOut, cmmOut1);
	cmmOut.iWrite("D:/Convolution/Convolution/out3.mfd");

	// 5x5
	cmmWin.iRead("D:/Convolution/Convolution/blur5.mfd");

	dStart = omp_get_wtime();
	iConvolution(cmmIn, cmmWin, cmmOut);
	dEnd = omp_get_wtime();
	printf("5x5 - %lg sec.\n", dEnd - dStart);

	dStart = omp_get_wtime();
	iConvolution_v4(cmmIn, cmmWin, cmmOut1);
	dEnd = omp_get_wtime();
	comparison(cmmOut, cmmOut1);
	printf("5x5 - %lg sec.\n", dEnd - dStart);

	cmmOut.iWrite("D:/Convolution/Convolution/out5.mfd");

	// 7x7
	cmmWin.iRead("D:/Convolution/Convolution/blur7.mfd");

	dStart = omp_get_wtime();
	iConvolution(cmmIn, cmmWin, cmmOut);
	dEnd = omp_get_wtime();
	printf("7x7 - %lg sec.\n", dEnd - dStart);


	dStart = omp_get_wtime();
	iConvolution_v2(cmmIn, cmmWin, cmmOut);
	dEnd = omp_get_wtime();
	printf("7x7 - %lg sec.\n", dEnd - dStart);
	cmmOut.iWrite("D:/Convolution/Convolution/out7.mfd");

	printf("Press any key...\n");
	_getch();
	return 0;
}

