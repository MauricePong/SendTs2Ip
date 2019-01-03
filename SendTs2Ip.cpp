// SendTs2Ip.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SourceTsFile.h"
#include "Rtp.h"

Udp* g_udp;
Rtp* g_rtp;
Source_TsFile* g_sourct;
int delayms = 0;

void GetTsPacketData(void* p_user_data)
{
	BYTE*	p_data;
	DWORD	data_size;
	
	if (!g_sourct->GetTsPacketData(p_data, data_size))
		return;
	
	if (g_udp){
		g_udp->Write(p_data, (UINT16)data_size);
	}
	else{
		g_rtp->Write(p_data, (UINT16)data_size, 0x21, GetTickCount());
	}
	Sleep(delayms);
}

BOOL ReadStdIn(int& i)
{
	int c;
	string text;

	while(TRUE)
	{
		c = getchar();
		if ( c == '\n' || c == EOF)
		{
			if (text.length() == 0)
				return FALSE;

			break;
		}
		text += (char)c;
	}	

	i = atoi( text.c_str());
	return TRUE;
}

UINT ReadStdIn(string_t& text)
{
	int i,c;
	string s_read;

	i = 0;
	text.clear();

	while(TRUE)
	{
		c = getchar();
		if ( c == '\n' || c == EOF)
			break;
		s_read += (char)c;
		i++;
	}
	text = a2t(s_read);
	return i;
}

int main(int argc, char* argv[])
{
	string_t	ts_file_path;
	string		bind_ip;
	INT			bind_port;
	string		target_ip;
	INT			target_port;
	INT			mtu;
	BOOL		is_rtp;
	int         tscout;

	vector<string> vIPList;
	if ( Socket::GetLocalIPList(vIPList) == FALSE)
	{
		printf("No ip address!\n");
		return 0;
	}

	cout << "*******************************\n" << endl;
	cout << "TBS STREAMING v1.0" << endl;
	cout << "*******************************\n\n" << endl;

	cout << "please input TS streaming address:" << endl;
	ReadStdIn(ts_file_path);

	string_t msg, c;
	INT i;
	msg = _T("\nSelect Bind IP Address (");
	for ( i = 0; i < vIPList.size(); i++) 
	{
		st_sprintf(&c, _T("%d"), i);
		msg += c;
	}
	msg += _T(", default index: 0) :");
	cout << t2a(msg) << endl;

	for ( i = 0; i < vIPList.size(); i++) 
	{
		cout << i << " " << vIPList[i] << endl;
	}
	if (ReadStdIn(i) == FALSE || i > vIPList.size())
		i = 0;
	bind_ip = vIPList[i];

	cout << "\nInput Bind Port (default 0) :" << endl;
	if (ReadStdIn(bind_port) == FALSE)
		bind_port = 0;

	cout << "\nInput Target IP Address (dafault 239.0.0.1):" << endl;
	if (ReadStdIn(msg) == 0)
		target_ip = "239.0.0.1";
	else
		target_ip = t2a(msg);

	cout << "\nInput Target Port (dafault 1234):" << endl;
	if(ReadStdIn(target_port) == FALSE)
		target_port = 1234;

	cout << "\nInput MTU (dafault 1482):" << endl;
	if(ReadStdIn(mtu) == FALSE)
		mtu = 1482;

	is_rtp = 1;
	cout << "\nInput Protocol, 0 UDP 1 RTP (dafault 1 RTP):" << endl;
	if(ReadStdIn(is_rtp) == FALSE)
		is_rtp = 1;

	tscout = 7;
	cout << "\nInput ts size = 188*n, (dafault n = 7):" << endl;
	if (ReadStdIn(tscout) == FALSE)
		tscout = 7;

	delayms = 0;
	cout << "\nInput send ts delayms time, (dafault 0):" << endl;
	if (ReadStdIn(delayms) == FALSE)
		delayms = 0;

	
	
	g_udp = NULL;
	g_rtp = NULL;

	if (is_rtp)	{
		g_rtp = new Rtp(mtu);
		g_rtp->Open(bind_ip, bind_port);
		g_rtp->Connect(target_ip, target_port);
	}
	else {
		g_udp = new Udp(mtu);
		g_udp->Open(bind_ip, bind_port);
		g_udp->Connect(target_ip, target_port);
	}
	
	g_sourct = new Source_TsFile();
	g_sourct->SetGetTsPacketDataCB(GetTsPacketData, NULL, TS_PACKET_SIZE_MIN*tscout);
	
	do
	{
		if (g_sourct->Open(ts_file_path.c_str()) == FALSE)
		{
			printf("Open ts file error :%s", t2a(ts_file_path).c_str());
			break;
		}

		cout << "\nIs working.\nIf u want exit, Input any key :" << endl;
		cin >> i;

	}while(0);

	g_sourct->Close();

	if (g_udp)
		delete g_udp;
	if (g_rtp)
		delete g_rtp;
	delete g_sourct;

	return 0;
}

