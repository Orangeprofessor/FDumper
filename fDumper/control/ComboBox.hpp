#pragma once

#include "Control.hpp"

namespace ctrl
{
	class ComboBox : public Control
	{
	public:
		ComboBox(HWND hwnd = NULL)
			: Control(hwnd) {  }

		virtual inline int selection() const { return ComboBox_GetCurSel(m_hwnd); }
		virtual inline int selection(int index) { return ComboBox_SetCurSel(m_hwnd, index); }

		virtual inline int itemData(int index) const { return (int)ComboBox_GetItemData(m_hwnd, index); }
		virtual inline int itemData(int index, int data) { return (int)ComboBox_SetItemData(m_hwnd, index, data); }

		virtual inline void reset() { ComboBox_ResetContent(m_hwnd); }

		virtual int Add(const std::wstring& text, int data = 0)
		{
			auto idx = ComboBox_AddString(m_hwnd, text.c_str());
			ComboBox_SetItemData(m_hwnd, idx, data);

			return idx;
		}

		virtual int Add(const std::string& text, int data = 0)
		{
			auto idx = (int)SendMessageA(m_hwnd, CB_ADDSTRING, 0, (LPARAM)text.c_str());
			ComboBox_SetItemData(m_hwnd, idx, data);

			return idx;
		}

		virtual std::wstring itemText(int index) const
		{
			wchar_t buf[512] = { 0 };
			ComboBox_GetLBText(m_hwnd, index, buf);

			return buf;
		}

		virtual void modifyItem(int index, const std::wstring& text, int data = 0)
		{
			auto oldData = itemData(index);
			if (data == 0)
				data = oldData;

			ComboBox_DeleteString(m_hwnd, index);
			index = ComboBox_InsertString(m_hwnd, index, text.c_str());
			ComboBox_SetItemData(m_hwnd, index, data);
		}

		virtual std::wstring selectedText() const
		{
			wchar_t buf[512] = { 0 };
			ComboBox_GetText(m_hwnd, buf, ARRAYSIZE(buf));

			return buf;
		}

		virtual inline void selectedText(const std::wstring& text) { ComboBox_SetText(m_hwnd, text.c_str()); }
	};
}