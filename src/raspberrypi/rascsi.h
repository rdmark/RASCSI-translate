//---------------------------------------------------------------------------
//
//	SCSI Target Emulator RaSCSI (*^..^*)
//	for Raspberry Pi
//	Powered by XM6 TypeG Technology.
//
//	Copyright (C) 2016-2021 GIMONS(Twitter:@kugimoto0715)
//
//	[ 共通定義 ]
//
//---------------------------------------------------------------------------

#if !defined(rascsi_h)
#define rascsi_h

//---------------------------------------------------------------------------
//
//	VERSION
//
//---------------------------------------------------------------------------
#define VERSION 0x0152

//---------------------------------------------------------------------------
//
//	RASCSI
//
//---------------------------------------------------------------------------
#define RASCSI VERSION

//---------------------------------------------------------------------------
//
//	IDマクロ
//
//---------------------------------------------------------------------------
#define MAKEID(a, b, c, d)	((DWORD)((a<<24) | (b<<16) | (c<<8) | d))

//---------------------------------------------------------------------------
//
//	オプション機能
//
//---------------------------------------------------------------------------
#define USE_BRIDGE_NET	1
#define USE_BRIDGE_FS	1

//---------------------------------------------------------------------------
//
//	クラス宣言
//
//---------------------------------------------------------------------------
class Fileio;
										// ファイル入出力
class Disk;
										// SASI/SCSIディスク
class Filepath;
										// ファイルパス

#endif	// rascsi_h
