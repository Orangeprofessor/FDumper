#pragma once

#include "Control.hpp"

#include <initializer_list>

namespace ctrl
{
	class ListView : public Control
	{
	public:
		ListView(HWND hwnd = NULL)
			: Control(hwnd) {  }

		virtual int AddColumn(const std::wstring& name, int width, int iSubItem = 0)
		{
			LVCOLUMNW lvc = { 0 };

			lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.pszText = (LPWSTR)name.c_str();
			lvc.iSubItem = iSubItem;
			lvc.cx = width;

			return ListView_InsertColumn(m_hwnd, iSubItem, &lvc);
		}

		virtual int AddItem(const std::wstring& text, LPARAM lParam, const std::initializer_list<std::wstring>& args = {})
		{
			LVITEMW lvi = { 0 };

			lvi.mask = LVIF_TEXT | LVIF_PARAM;

			lvi.pszText = (LPWSTR)text.c_str();
			lvi.cchTextMax = static_cast<int>(text.length()) + 1;
			lvi.lParam = lParam;
			lvi.iItem = ListView_GetItemCount(m_hwnd);

			int pos = ListView_InsertItem(m_hwnd, &lvi);

			for (size_t i = 0; i < args.size(); i++)
				ListView_SetItemText(m_hwnd, pos, (int)(i + 1), (LPWSTR)(args.begin() + i)->c_str());

			return pos;
		}

		virtual int AddImage(const std::wstring& text, LPARAM lParam = NULL)
		{
			LVITEMW lvi = { 0 };

			lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

			lvi.lParam = lParam;
			lvi.pszText = (LPWSTR)text.c_str();
			lvi.cchTextMax = static_cast<int>(text.length()) + 1;
			lvi.iItem = lvi.iImage = ListView_GetItemCount(m_hwnd);
		
			return ListView_InsertItem(m_hwnd, &lvi);
		}

		virtual std::wstring itemText(int idx, int iSubItem = 0)
		{
			wchar_t buf[256] = { 0 };
			ListView_GetItemText(m_hwnd, idx, iSubItem, buf, ARRAYSIZE(buf));

			return buf;
		}

		virtual void setText(const std::wstring& text, int idx, int iSubItem = 0)
		{
			ListView_SetItemText(m_hwnd, idx, iSubItem, (LPWSTR)text.c_str());
		}

		virtual inline void RemoveItem(int idx) { ListView_DeleteItem(m_hwnd, idx); }

		virtual inline int selection() const { return ListView_GetNextItem(m_hwnd, -1, LVNI_SELECTED); }

		virtual inline void reset() { ListView_DeleteAllItems(m_hwnd); }
	};
}
