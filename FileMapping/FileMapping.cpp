#include "stdafx.h"
#include "FileMapping.h"
#include <tchar.h>
#include <cstdint>

///
/// ファイルマッピングオブジェクトを管理するクラス。
///
class CFileMappingImpl : public IFileMapping
{
private:
	enum
	{
		/// オブジェクト名の長さ。
		NAME_LENGTH = 63
	};

	///
	/// ファイルマッピングオブジェクトのヘッダ情報。
	///
	struct Head
	{
		/// ファイルマッピングオブジェクトのサイズ。
		DWORD Size;

		/// ヘッダ情報の予備領域。
		BYTE Reserves[16 - sizeof(DWORD)];
	};

private:
	/// ファイルマッピングのオブジェクト名。
	TCHAR m_FileMappingName[NAME_LENGTH + 1];

	/// ファイルマッピングのオブジェクトハンドル。
	HANDLE m_FileMappingHandle;

	/// ミューテックスのオブジェクト名。
	TCHAR m_MutexName[NAME_LENGTH + 1];

	/// ミューテックスのオブジェクトハンドル。
	HANDLE m_MutexHandle;

	/// マップされたビューの開始アドレス。
	void* m_Ptr;

	/// エラーコード。
	DWORD m_ErrorCode;

public:
	///
	/// CFileMappingImpl オブジェクトをコンストラクタ。
	///
	CFileMappingImpl()
		: IFileMapping()
		, m_FileMappingName()
		, m_FileMappingHandle(NULL)
		, m_MutexName()
		, m_MutexHandle(NULL)
		, m_Ptr(0)
		, m_ErrorCode(ERROR_SUCCESS)
	{
		::SecureZeroMemory(this->m_FileMappingName, sizeof(this->m_FileMappingName));
		::SecureZeroMemory(this->m_MutexName, sizeof(this->m_MutexName));
	}

	///
	/// CFileMappingImpl オブジェクトをデストラクタ。
	///
	virtual ~CFileMappingImpl()
	{
		if (this->m_Ptr)
		{
			::UnmapViewOfFile(this->m_Ptr);

			this->m_Ptr = NULL;
		}

		if (this->m_FileMappingHandle)
		{
			::CloseHandle(this->m_FileMappingHandle);

			this->m_FileMappingHandle = NULL;
		}

		if (this->m_MutexHandle)
		{
			::CloseHandle(this->m_MutexHandle);

			this->m_MutexHandle = NULL;
		}
	}

	///
	/// ファイルマッピングオブジェクトを作成または開きます。
	///
	/// @param size 作成するファイルマップオブジェクトのサイズ。
	/// @return 関数が成功すると true、関数が失敗すると false が返ります。
	///         エラー情報を取得するには GetErrorCode 関数を使います。
	///
	virtual	bool Create(size_t size)
	{
		this->m_ErrorCode = ERROR_SUCCESS;

		this->m_FileMappingHandle = ::CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			static_cast<DWORD>(size + sizeof(CFileMappingImpl::Head)),
			this->m_FileMappingName);

		// CreateFileMapping 成功時もエラーコードが変更されるため
		// この時点でエラーコードを取得しておく。
		this->m_ErrorCode = ::GetLastError();

		if (this->m_FileMappingHandle)
		{
			this->m_Ptr = ::MapViewOfFile(this->m_FileMappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

			if (this->m_Ptr)
			{
				if (this->m_ErrorCode == ERROR_SUCCESS)
				{
					::SecureZeroMemory(this->m_Ptr, size + sizeof(CFileMappingImpl::Head));

					static_cast<CFileMappingImpl::Head*>(this->m_Ptr)->Size = static_cast<DWORD>(size);
				}

				this->m_MutexHandle = ::CreateMutex(NULL, FALSE, this->m_MutexName);

				if (this->m_MutexHandle)
				{
					return true;
				}
			}
		}

		this->m_ErrorCode = ::GetLastError();

		return false;
	}

	///
	/// 既に存在するファイルマップオブジェクトを開きます。
	///
	/// @return 関数が成功すると true、関数が失敗すると false が返ります。
	///         エラー情報を取得するには GetErrorCode 関数を使います。
	///
	virtual	bool Open()
	{
		this->m_ErrorCode = ERROR_SUCCESS;

		this->m_FileMappingHandle = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, this->m_FileMappingName);

		if (this->m_FileMappingHandle)
		{
			this->m_Ptr = ::MapViewOfFile(this->m_FileMappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);

			if (this->m_Ptr)
			{
				this->m_MutexHandle = ::CreateMutex(NULL, FALSE, this->m_MutexName);

				if (this->m_MutexHandle)
				{
					return true;
				}
			}
		}

		this->m_ErrorCode = ::GetLastError();

		return false;
	}

	///
	/// ファイルマッピングのオブジェクト名を取得します。
	///
	virtual const TCHAR* GetName() const
	{
		return this->m_FileMappingName;
	}

	///
	/// ファイルマッピングのオブジェクト名を設定します。
	///
	virtual void SetName(const TCHAR* name)
	{
		_tcsncpy_s(this->m_FileMappingName, name, NAME_LENGTH);

		_sntprintf_s(this->m_MutexName, NAME_LENGTH, _T("%s%s"), name, _T("Mutex"));
	}

	///
	/// ファイルマッピングオブジェクトのサイズを取得します。
	///
	virtual size_t GetSize() const
	{
		return static_cast<CFileMappingImpl::Head*>(this->m_Ptr)->Size;
	}

	///
	/// ビューのマッピングを開始したアドレスを取得します。
	///
	virtual void* GetPtr()
	{
		return static_cast<CFileMappingImpl::Head*>(this->m_Ptr) + 1;
	}

	///
	/// ビューのマッピングを開始したアドレスを取得します。
	///
	virtual const void* GetPtr() const
	{
		return static_cast<CFileMappingImpl::Head*>(this->m_Ptr) + 1;
	}

	///
	/// マッピングを開始したアドレスのアクセス制御を開始します。
	///
	/// @return 関数が成功すると true、関数が失敗すると false が返ります。
	///         エラー情報を取得するには GetErrorCode 関数を使います。
	///
	virtual bool Lock()
	{
		this->m_ErrorCode = ERROR_SUCCESS;

		if (this->m_MutexHandle)
		{
			if (::WaitForSingleObject(this->m_MutexHandle, INFINITE) != WAIT_OBJECT_0)
			{
				this->m_ErrorCode = ::GetLastError();

				return false;
			}
		}

		return true;
	}

	///
	/// マッピングを開始したアドレスのアクセス制御を解放します。
	///
	/// @return 関数が成功すると true、関数が失敗すると false が返ります。
	///         エラー情報を取得するには GetErrorCode 関数を使います。
	///
	virtual bool Unlock()
	{
		this->m_ErrorCode = ERROR_SUCCESS;

		if (this->m_MutexHandle)
		{
			if (::ReleaseMutex(this->m_MutexHandle) == FALSE)
			{
				this->m_ErrorCode = ::GetLastError();

				return false;
			}
		}

		return true;
	}

	///
	/// エラーコードを取得します。
	///
	virtual DWORD GetErrorCode() const
	{
		return this->m_ErrorCode;
	}
};

///
/// IFileMapping を実装したオブジェクトを生成します。
///
IFileMapping* IFileMapping::CreateInstance()
{
	return new CFileMappingImpl();
}
