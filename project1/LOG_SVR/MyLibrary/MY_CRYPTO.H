/*================================================================================
 by ozzywow
=================================================================================*/




#pragma once


// compile option

//_USE_DECRYPTION_
//_USE_ENCRYPTION_
//_USE_HEAVY_MEM_POOL_
//_USE_LIGHT_MEM_

#define _SERVER_VERSION_

#ifdef _SERVER_VERSION_
	#define _USE_DECRYPTION_
#elif defined _CLIENT_VERSION_	
	#define _USE_ENCRYPTION_	
	#define _USE_LIGHT_MEM_
#elif defined _DUMMY_VERSION_	
	#define _USE_ENCRYPTION_
	#define _USE_HEAVY_MEM_POOL_	
#endif


//#define _USE_HEADER_
#ifdef _USE_HEADER_
	struct _sHeader {
		short		sLength : 12 ;		// -2048 ~ 2047
		short		sCrypto : 2 ;		// 0 : false , 나머지는 true
		short		sCompressed : 2 ;	//0 : false , 나머지는 true
	};
#endif


#ifndef IN
#define IN
#define OUT
#endif


#define KEYBOX_FILE			"crypto.dat"	// 키박스 파일 입니다.
#define KEYVALLENTH			16			// 랜덤생성키의 최대 크기
#define KEY_RANGE			0xff		// 키 갯수..

/*================================================================================
 CLASS NAME : _j_Crypto 
 EXP		: 데이타 암호화 / 복호화
 AUTHOR		: ozzywow
 DATE		: 2003.11. ,   SE : 2005,4,07
=================================================================================*/


class _j_Crypto
{

private:	
	
	DWORD	m_dwKeyLength;									// Key 의 길이	
	char m_cKeyBox[KEY_RANGE][KEYVALLENTH];					// Key 들이 들어있는 BOX	
	
	int		m_iBufMaxSize ;
	int		m_iBufCurSet ;
	

public:		
	_j_Crypto::_j_Crypto( size_t bufSize ) : m_iBufMaxSize( bufSize ), m_iBufCurSet(0)
	{
		memset(m_cKeyBox,0,sizeof(m_cKeyBox));		
	}
	_j_Crypto::~_j_Crypto()
	{
	}

	
	// 초기화 (키값 생성)
	// pathName : 키 맵으로 사용할 파일 이름..
	// 클래스를 생성할깨 초기 좌표값 (x,y) 를 넣어서 초기화해 주세요.
	bool _j_Crypto::Init(const char * pathName)
	{
		if( pathName )
		{
			//키 파일 읽어오기..
			FILE *	stream = NULL;
			try{
				stream = fopen(pathName,"rb");
			}
			catch (...) {		
				fclose(stream);
				return false;
			}
			
			if(stream == NULL) {
				//fclose(stream);
				return false;
			}		
			
			//while(!feof(stream)) {
			//	if(fread(&m_cKeyBox[0][0][0], sizeof(char), sizeof(m_cKeyBox), stream) == NULL) ;//break;
			//}
			memset( m_cKeyBox, 0, sizeof(m_cKeyBox) ) ;

			fread(m_cKeyBox, sizeof(m_cKeyBox) , 1, stream) ;
			
			if(fclose(stream) != 0) return false;
			
			// 키 파일 읽기 끝
		}
		return true ;
	}
	
#ifdef _USE_ENCRYPTION_
	//암호화//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 이 함수는 데이타를 암호화 한다.   									//
	// 함수 : Encryption(char * pData, USHORT usDataLen)					//
	// 설명 : 키와 비트 연산을 하고 대각선형 전치 조합으로 데이타를 혼돈시켰음//
	//////////////////////////////////////////////////////////////////////////
	inline bool _j_Crypto::Encryption( IN OUT char * pData, const USHORT usDataLen, const u_char key )
	{
	
		// Get key string
		char * cValKey ;
		UINT * uiValKey ;
		cValKey = GetKey( key ) ;		
		uiValKey = (UINT*)cValKey ;	
		

		try {

			int x, i,j,k,l ;
			
			UINT * puiData = (UINT *) pData ;						// 'int *' type data 로 변환		

			j = 0 ;							// line step
			k = usDataLen / KEYVALLENTH ;	// line count
			l = usDataLen % KEYVALLENTH ;	// char count (잔텡이 데이타)			
		
			
			// line data block Encrypto
			for( i = 0, x = 0, j = 0 ; i < k ; i++, x += 4, j += 16 ) {	// Key 와 pData 를 전치한다.				
				puiData[x+0] = uiValKey[0] ^ puiData[x+0] ;
				puiData[x+1] = ~(uiValKey[2] ^ puiData[x+1]) ;
				puiData[x+2] = uiValKey[1] ^ puiData[x+2] ;
				puiData[x+3] = ~(uiValKey[3] ^ puiData[x+3]) ;
			}
			
			// char data Encrypto (잔텡이 데이타)
			//j += KEYVALLENTH ;
			for(int h = 0 ; h < l ; h++ ) {
				pData[j+h] = ~(cValKey[h] ^ pData[j+h]) ;
			}
			
		} // end try
		catch (...) {
			return false ;
		}	
		return true ;
		
	}
#endif

#ifdef _USE_DECRYPTION_
	//복호화//////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// 이 함수는 데이타를 복호화 한다.   									//
	// 함수 : Decryption(char * pData, USHORT usDataLen)						//
	// 설명 : 키와 비트 연산을 하고 대각선형 전치 조합으로 데이타를 혼돈시켰음//
	//////////////////////////////////////////////////////////////////////////
	inline bool _j_Crypto::Decryption(char * pData, const USHORT usDataLen, const u_char key )
	{	

		// Get key string
		char * cValKey ;
		UINT * uiValKey ;
		cValKey = GetKey( key ) ;		
		uiValKey = (UINT*)cValKey ;	

		try {	
			
			int x, i,j,k,l ;
			
			UINT * puiData = (UINT *) pData ;						// 'int *' type data 로 변환		
			
			j = 0 ;							// line step
			k = usDataLen / KEYVALLENTH ;	// line count
			l = usDataLen % KEYVALLENTH ;	// char count (잔텡이 데이타)			
			
			
			// line data block Encrypto
			for( i = 0, x = 0, j = 0 ; i < k ; i++, x += 4, j += 16 ) {	// Key 와 pData 를 전치한다.				
				puiData[x+0] = uiValKey[0] ^ puiData[x+0] ;
				puiData[x+1] = ~(uiValKey[2] ^ puiData[x+1]) ;
				puiData[x+2] = uiValKey[1] ^ puiData[x+2] ;
				puiData[x+3] = ~(uiValKey[3] ^ puiData[x+3]) ;
			}
			
			// char data Encrypto (잔텡이 데이타)
			//j += KEYVALLENTH ;
			for(int h = 0 ; h < l ; h++ ) {
				pData[j+h] = ~(cValKey[h] ^ pData[j+h]) ;
			}

		}// try
		catch (...) {
			return false ;
		}
		return true ;		
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// 이 함수는 데이타를 암/복호화 한다.									//
	// 함수 : Xor(char * pData, USHORT usDataLen)	인수 : 문자열 포인터(plantext)	//
	// 설명 : 문자열을 입력된 키와 단순한 XOR 연산을 수행한다.              //
	//////////////////////////////////////////////////////////////////////////
	inline char * _j_Crypto::Xor(char *pData, USHORT usDataLen, const u_char key )
	{	
		char * cValKey ;
		cValKey = GetKey( key ) ;		
		
		for(USHORT i = 0; i < usDataLen; i++){
			pData[i]=~ pData[i]^cValKey[i % KEYVALLENTH];
		}

		return pData;		
	}	

	
	inline u_char _j_Crypto::checksum(u_char *buffer, int size) {
		
		unsigned long cksum=0;
		
		while(size >1) {
			cksum+=*buffer++;
			size -=sizeof(u_char);
		}
		
		if(size ) {
			cksum += *(u_char*)buffer;
		}
		
		cksum = (cksum >> 16) + (cksum & 0xffff);
		cksum += (cksum >>16);
		return (u_char)(~cksum);
	}


private:
	inline char * _j_Crypto::GetKey(const u_char key , const char * pathName = NULL )	
	{		
		return m_cKeyBox[key % KEY_RANGE];
	}	
};



struct _Tail {
	u_char		crc ;
	u_char		seq ;
};

struct _Encapsule_info {	
	char *		buf ;
	u_short		length ;
};

struct _Decapsule_info {	
	char *		buf ;
	u_short		length ;
	u_char		seq ;
};


#ifdef _USE_HEAVY_MEM_POOL_
	#define _CAPSULE_BUF_SIZE_		1048576 * 5
#elif defined _USE_LIGHT_MEM_
	#define _CAPSULE_BUF_SIZE_		4096
#else
	#define _CAPSULE_BUF_SIZE_		0
#endif

#define _TAIL_SIZE_				2

class CCapsulateCrypto : public _j_Crypto
{

private:
	char *	m_pCapsule_buf ;
	int		m_iBufMaxSize ;
	int		m_iBufCurSet ;
	u_char	m_ucSeq ;

#ifdef _USE_HEAVY_MEM_POOL_
	CRITICAL_SECTION	m_cs ;
	inline void lock() { EnterCriticalSection( &m_cs ) ; }
	inline void unlock() { LeaveCriticalSection( &m_cs ) ; }	
#endif

public:

	CCapsulateCrypto(size_t bufSize = _CAPSULE_BUF_SIZE_ ) : _j_Crypto(bufSize), m_iBufMaxSize(bufSize), m_iBufCurSet(0), m_ucSeq(0)
	{
#ifdef _USE_HEAVY_MEM_POOL_

	#if (_WIN32_WINNT > 0x0403)
			if( FALSE == InitializeCriticalSectionAndSpinCount( &m_cs, (0x80000000)|2000 ) )
			{
				throw ;
			}			
	#else
			InitializeCriticalSection( &m_cs ) ;			
	#endif	
			m_pCapsule_buf = new char[bufSize] ;

#else if _USE_LIGHT_MEM_

			m_pCapsule_buf = new char[bufSize] ;

#endif			
	
	}



	~CCapsulateCrypto()
	{		
#ifdef _USE_HEAVY_MEM_POOL_
		DeleteCriticalSection( &m_cs ) ;
		delete [] m_pCapsule_buf ;
#else if _USE_LIGHT_MEM_
		delete [] m_pCapsule_buf ;
#endif
	}

	inline void InitSeqNum( u_char seq ) { m_ucSeq = seq ; }
	
#ifdef _USE_ENCRYPTION_
	inline _Encapsule_info Encapsulate( char * packet, u_char key = 0  ) 
	{
		if( key == 0 ) 	{ key = m_ucSeq++ ; }
		_sHeader * pHeader = (_sHeader*)packet ;
		_Encapsule_info capsuleInfo ;
		
#ifdef _USE_HEAVY_MEM_POOL_
		lock() ;
		if( pHeader->sLength + _TAIL_SIZE_ >= m_iBufMaxSize - m_iBufCurSet )
		{
			m_iBufCurSet = 0 ;
		}
		memcpy( &m_pCapsule_buf[m_iBufCurSet], packet, pHeader->sLength ) ;
		packet = &m_pCapsule_buf[m_iBufCurSet] ;
		m_iBufCurSet += pHeader->sLength + _TAIL_SIZE_ ;
		unlock() ;
#else _USE_LIGHT_MEM_
		if( pHeader->sLength + _TAIL_SIZE_ > m_iBufMaxSize )
		{
			capsuleInfo.buf = NULL ;
			capsuleInfo.length = 0 ;
			return capsuleInfo ;
		}
		
		memcpy( m_pCapsule_buf, packet, pHeader->sLength ) ;
		packet = m_pCapsule_buf ;		
#endif
		
		//  Add tail
		pHeader = (_sHeader*)packet ;
		_Tail * pTail = (_Tail*)&packet[pHeader->sLength] ;
		pTail->crc = checksum( (u_char*)packet, pHeader->sLength ) ;
		pTail->seq = key ;
		pHeader->sLength +=  _TAIL_SIZE_ ;		
		
		// encryption
		if( pHeader->sCrypto == 1 )
		{
			if( Encryption( (char*)&packet[2], pHeader->sLength, key ) == false )			
			{
				capsuleInfo.buf = NULL ;
				capsuleInfo.length = 0 ;
				return capsuleInfo ;
			}
		}
		
		capsuleInfo.buf = packet ;
		capsuleInfo.length = pHeader->sLength ;
		
		return capsuleInfo ;
	}
#endif

#ifdef _USE_DECRYPTION_
	inline _Decapsule_info Decapsulate( char * packet, u_char key = 0  ) 
	{
		if( key == 0 ) 	{ key = m_ucSeq ; }
		_sHeader * pHeader = (_sHeader*)packet ;
		_Decapsule_info	decapsule_info ={0};
		if( pHeader->sCrypto == 1 )
		{
			if( Decryption( (char*)&packet[2], pHeader->sLength, key ) == false )
			{
				decapsule_info.buf = NULL ;
				decapsule_info.length = 0 ;
				return decapsule_info ;
			}
		}

		pHeader->sLength -= _TAIL_SIZE_ ;
		_Tail * pTail = (_Tail*)&packet[pHeader->sLength] ;
		if( pTail->crc == checksum( (u_char*)packet, pHeader->sLength ) )
		{
			decapsule_info.buf = packet ;
			decapsule_info.length = pHeader->sLength ;			
		}
		else
		{
			decapsule_info.buf = NULL ;
			decapsule_info.length = 0 ;
		}
		decapsule_info.seq = pTail->seq ;

		return decapsule_info ;
	}
#endif

};


