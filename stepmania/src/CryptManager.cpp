#include "global.h"
#include "CryptManager.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageFile.h"

#include "crypto/CryptRSA.h"
#include "crypto/CryptRand.h"

static const CString SIGNATURE_APPEND = ".sig.rsa";
static const CString PRIVATE_KEY_PATH = "Data/private.key.rsa";
static const CString PUBLIC_KEY_PATH = "Data/public.key.rsa";
static const int KEY_LENGTH = 1024;

CryptManager*	CRYPTMAN	= NULL;	// global and accessable from anywhere in our program

CryptManager::CryptManager()
{
	if( !PREFSMAN->m_bSignProfileData )
		return;

	//
	// generate keys if none are available
	//
	if( !DoesFileExist(PRIVATE_KEY_PATH) || !DoesFileExist(PUBLIC_KEY_PATH) )
	{
		LOG->Warn( "Keys missing.  Generating new keys" );
		GenerateRSAKey( KEY_LENGTH, PRIVATE_KEY_PATH, PUBLIC_KEY_PATH, "aoksdjaksd" );
		FlushDirCache();
	}
}

CryptManager::~CryptManager()
{

}

void CryptManager::GenerateRSAKey( unsigned int keyLength, CString privFilename, CString pubFilename, CString seed )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	random_init();
	random_add_noise( seed );

	RSAKey key;
	key.Generate( keyLength );

	RageFile out;

	CString sPublic;
	key.PublicBlob( sPublic );
	if( !out.Open( pubFilename, RageFile::WRITE ) )
		RageException::Throw( "Error opening %s: %s", pubFilename.c_str(), out.GetError().c_str() );
	out.Write( sPublic );
	out.Close();

	CString sPrivate;
	key.PrivateBlob( sPrivate );
	if( !out.Open( privFilename, RageFile::WRITE ) )
		RageException::Throw( "Error opening %s: %s", privFilename.c_str(), out.GetError().c_str() );
	out.Write( sPrivate );
	out.Close();
}

void CryptManager::SignFile( CString sPath )
{
	LOG->Trace("SignFile(%s)", sPath.c_str());
	ASSERT( PREFSMAN->m_bSignProfileData );

	if( !IsAFile(PRIVATE_KEY_PATH) )
		return;

	if( !IsAFile(sPath) )
		return;

	const CString sig = GetFileSignature( sPath );

	RageFile out;
	const CString sSignatureFilename = sPath + SIGNATURE_APPEND;
	if( !out.Open( sSignatureFilename, RageFile::WRITE ) )
		RageException::Throw( "Error opening %s: %s", sSignatureFilename.c_str(), out.GetError().c_str() );
	out.Write( sig );
}

bool CryptManager::VerifyFile( CString sPath )
{
	LOG->Trace("VerifyFile(%s)", sPath.c_str());
	ASSERT( PREFSMAN->m_bSignProfileData );

	if( !IsAFile(PUBLIC_KEY_PATH) )
		return false;

	const CString sSignatureFilename = sPath + SIGNATURE_APPEND;
	if( !IsAFile(sSignatureFilename) )
		return false;

	CString sig;
	{
		RageFile in;
		if( !in.Open( sSignatureFilename, RageFile::READ ) )
			RageException::Throw( "Error opening %s: %s", sSignatureFilename.c_str(), in.GetError().c_str() );
		in.Read( sig );
	}

	return VerifyFile( sPath, sig );
}

CString CryptManager::GetFileSignature( CString sPath )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	if( !IsAFile(PRIVATE_KEY_PATH) )
		return "";

	if( !IsAFile(sPath) )
		return "";

	CString data;
	{
		RageFile in;
		if( !in.Open( sPath, RageFile::READ ) )
			RageException::Throw( "Error opening %s: %s", sPath.c_str(), in.GetError().c_str() );
		in.Read( data );
	}

	RSAKey key;
	{
		RageFile keyfile;
		if( !keyfile.Open( PRIVATE_KEY_PATH ) )
			RageException::Throw( "Error opening %s: %s", PRIVATE_KEY_PATH.c_str(), keyfile.GetError().c_str() );
		CString private_blob;
		keyfile.Read( private_blob );
		key.LoadFromPrivateBlob( private_blob );
	}

	CString sig;
	key.Sign( data, sig );

	return sig;
}

bool CryptManager::VerifyFile( CString sPath, CString sSignature )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	CString sPubFilename = PUBLIC_KEY_PATH;
	CString sMessageFilename = sPath;

	if( !IsAFile(sPubFilename) )
		return false;

	CString data;
	{
		RageFile in;
		if( !in.Open( sPath, RageFile::READ ) )
			RageException::Throw( "Error opening %s: %s", sPath.c_str(), in.GetError().c_str() );
		in.Read( data );
	}

	RSAKey key;
	{
		RageFile keyfile;
		if( !keyfile.Open( PRIVATE_KEY_PATH ) )
			RageException::Throw( "Error opening %s: %s", PRIVATE_KEY_PATH.c_str(), keyfile.GetError().c_str() );
		CString private_blob;
		keyfile.Read( private_blob );
		key.LoadFromPrivateBlob( private_blob );
	}

	return key.Verify( data, sSignature );
}


void CryptManager::DigestFile( CString fn )
{
	ASSERT( PREFSMAN->m_bSignProfileData );

//	MD5 md5;
//	HashFilter md5Filter(md5);
//
//	auto_ptr<ChannelSwitch> channelSwitch(new ChannelSwitch);
//	channelSwitch->AddDefaultRoute(md5Filter);
//	RageFileSource(filename, true, channelSwitch.release());
//
//	HexEncoder encoder(new RageFileSink(cout), false);
//	cout << "\nMD5: ";
//	md5Filter.TransferTo(encoder);
}

CString CryptManager::GetPublicKeyFileName()
{
	ASSERT( PREFSMAN->m_bSignProfileData );

	return PUBLIC_KEY_PATH;
}
