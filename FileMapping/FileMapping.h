///
/// @file FileMapping.h
///
/// @author Y.Koseki
///
#pragma once
#ifndef _FILE_MAPPING_
#define _FILE_MAPPING_
#include <windows.h>

///
/// ファイルマッピングオブジェクトを管理するクラスのインターフェースを定義します。
///
class IFileMapping
{
protected:
	///
	/// IFileMapping オブジェクトをコンストラクタ。
	///
	IFileMapping() { }

public:
	///
	/// IFileMapping オブジェクトをデストラクタ。
	///
	virtual ~IFileMapping() { }

	///
	/// IFileMapping を実装したオブジェクトを生成します。
	///
	static IFileMapping* CreateInstance();

	///
	/// ファイルマッピングオブジェクトを作成または開きます。
	///
	/// @param size 作成するファイルマップオブジェクトのサイズ。
	/// @return 関数が成功すると true、関数が失敗すると false が返ります。
	///         エラー情報を取得するには GetErrorCode 関数を使います。
	///
	virtual bool Create(size_t size) = 0;

	///
	/// 既に存在するファイルマップオブジェクトを開きます。
	///
	/// @return 関数が成功すると true、関数が失敗すると false が返ります。
	///         エラー情報を取得するには GetErrorCode 関数を使います。
	///
	virtual bool Open() = 0;

	///
	/// ファイルマッピングのオブジェクト名を取得します。
	///
	virtual const TCHAR* GetName() const = 0;

	///
	/// ファイルマッピングのオブジェクト名を設定します。
	///
	virtual void SetName(const TCHAR* name) = 0;

	///
	/// ファイルマッピングオブジェクトのサイズを取得します。
	///
	virtual size_t GetSize() const = 0;

	///
	/// ビューのマッピングを開始したアドレスを取得します。
	///
	virtual void* GetPtr() = 0;

	///
	/// ビューのマッピングを開始したアドレスを取得します。
	///
	virtual const void* GetPtr() const = 0;

	///
	/// マッピングを開始したアドレスのアクセス制御を開始します。
	///
	/// @return 関数が成功すると true、関数が失敗すると false が返ります。
	///         エラー情報を取得するには GetErrorCode 関数を使います。
	///
	virtual bool Lock() = 0;

	///
	/// マッピングを開始したアドレスのアクセス制御を解放します。
	///
	/// @return 関数が成功すると true、関数が失敗すると false が返ります。
	///         エラー情報を取得するには GetErrorCode 関数を使います。
	///
	virtual bool Unlock() = 0;

	///
	/// エラーコードを取得します。
	///
	virtual DWORD GetErrorCode() const = 0;
};

///
/// ファイルマッピングオブジェクトのアドレスにアクセスするクラス。
///
template <class T>
class CFileMappingPtr
{
private:
	/// ファイルマップの管理オブジェクト。
	IFileMapping* m_FileMapping;

	/// マップされたビューの開始アドレス。
	T* m_Ptr;

	/// 代入演算子
	IFileMapping& operator=(IFileMapping& obj) = delete;

public:
	///
	/// CFileMappingPtr オブジェクトをコンストラクタ。
	///
	CFileMappingPtr(IFileMapping* fileMapping)
		: m_FileMapping(NULL)
		, m_Ptr(NULL)
	{
		if (fileMapping->Lock())
		{
			this->m_FileMapping = fileMapping;
			this->m_Ptr = static_cast<T*>(this->m_FileMapping->GetPtr());
		}
	}

	///
	/// CFileMappingPtr オブジェクトをコピーコンストラクタ。
	///
	CFileMappingPtr(CFileMappingPtr& obj)
		: m_FileMapping(obj.m_FileMapping)
		, m_Ptr(obj.m_Ptr)
	{
		obj.m_FileMapping = NULL;
		obj.m_Ptr = NULL;
	}

	///
	/// CFileMappingPtr オブジェクトをデストラクタ。
	///
	virtual ~CFileMappingPtr()
	{
		this->m_FileMapping->Unlock();

		this->m_FileMapping = NULL;
		this->m_Ptr = NULL;
	}

	///
	/// ビューのマッピングを開始したアドレスを取得します。
	///
	T* operator->()
	{
		return this->m_Ptr;
	}

	///
	/// ビューのマッピングを開始したアドレスを取得します。
	///
	const T* operator->() const
	{
		return this->m_Ptr;
	}
};

///
/// ファイルマッピングオブジェクトを管理するクラス。
///
template <class T>
class CFileMapping
{
private:
	/// ファイルマップの管理オブジェクト。
	IFileMapping* m_FileMapping;

public:
	///
	/// CFileMapping オブジェクトをコンストラクタ。
	///
	CFileMapping(const TCHAR* name)
		: m_FileMapping(IFileMapping::CreateInstance())
	{
		this->m_FileMapping->SetName(name);
	}

	///
	/// CFileMapping オブジェクトをデストラクタ。
	///
	virtual ~CFileMapping()
	{
		delete this->m_FileMapping;

		this->m_FileMapping = NULL;
	}

	///
	/// ファイルマッピングのオブジェクト名を取得します。
	///
	const TCHAR* GetName() const
	{
		return this->m_FileMapping->GetName();
	}

	///
	/// ファイルマッピングのオブジェクト名を設定します。
	///
	void SetName(const TCHAR* name)
	{
		this->m_FileMapping->SetName(name);
	}

	///
	/// ファイルマッピングオブジェクトを作成または開きます。
	///
	/// @return 関数が成功すると true、関数が失敗すると false が返ります。
	///         エラー情報を取得するには GetErrorCode 関数を使います。
	///
	bool Create()
	{
		return this->m_FileMapping->Create(sizeof(T));
	}

	///
	/// 既に存在するファイルマップオブジェクトを開きます。
	///
	/// @return 関数が成功すると true、関数が失敗すると false が返ります。
	///         エラー情報を取得するには GetErrorCode 関数を使います。
	///
	bool Open()
	{
		return this->m_FileMapping->Open();
	}

	///
	/// ビューのマッピングを開始したアドレスを取得します。
	///
	CFileMappingPtr<T> GetPtr()
	{
		return CFileMappingPtr<T>(this->m_FileMapping);
	}

	///
	/// ビューのマッピングを開始したアドレスを取得します。
	///
	const CFileMappingPtr<T> GetPtr() const
	{
		return CFileMappingPtr<T>(this->m_FileMapping);
	}
};
#endif // _FILE_MAPPING_
