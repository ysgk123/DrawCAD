// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问
// https://go.microsoft.com/fwlink/?LinkId=238214.
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// DrawCADDoc.cpp: CDrawCADDoc 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "DrawCAD.h"
#endif

#include "DrawCADDoc.h"

#include <propkey.h>

#include "MainFrm.h"
#include "DrawCADView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDrawCADDoc

IMPLEMENT_DYNCREATE(CDrawCADDoc, CDocument)

BEGIN_MESSAGE_MAP(CDrawCADDoc, CDocument)
	//ON_COMMAND(ID_FILE_NEW, &CDrawCADDoc::OnFileNew)
	ON_COMMAND(ID_FILE_SAVE, &CDrawCADDoc::OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, &CDrawCADDoc::OnFileSaveAs)
	ON_COMMAND(ID_FILE_OPEN, &CDrawCADDoc::OnFileOpen)
	ON_COMMAND(ID_INSERT_FILE, &CDrawCADDoc::OnInsertFile)
	//撤销消息
	ON_COMMAND(ID_EDIT_UNDO, &CDrawCADDoc::OnEditUndo)
	ON_COMMAND(ID_EDIT_REDO, &CDrawCADDoc::OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &CDrawCADDoc::OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &CDrawCADDoc::OnUpdateEditRedo)
	ON_COMMAND(ID_EDIT_COPY, &CDrawCADDoc::OnEditCopy)
	ON_COMMAND(ID_EDIT_CUT, &CDrawCADDoc::OnEditCut)
//	ON_COMMAND(ID_EDIT_PASTE, &CDrawCADDoc::OnEditPaste)
	ON_COMMAND(ID_EDIT_SELECT_ALL, &CDrawCADDoc::OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &CDrawCADDoc::OnUpdateEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &CDrawCADDoc::OnUpdateEditCut)
//	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CDrawCADDoc::OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, &CDrawCADDoc::OnUpdateEditSelectAll)
END_MESSAGE_MAP()


// CDrawCADDoc 构造/析构

CDrawCADDoc::CDrawCADDoc() noexcept
{
	// TODO: 在此添加一次性构造代码
	m_LineColor = RGB(0, 0, 0);			//黑色
	m_CurLineColor = RGB(255, 255, 0);	//黄色
	m_BackColor = RGB(204, 204, 204);	//浅灰
	m_StateShow.m_r = 1;
	m_StateShow.m_dx = 0;
	m_StateShow.m_dy = 0;
	m_StateShow.m_bViewPoint = true;
	DrawMod = LineMode;
	m_LineWidth = 1;
	m_nZoom = 1.2;
	m_MoveStep = 1;
	m_LineStyle = 0;
	m_bSnapEnabled = TRUE;      // 默认开启吸附


	// 初始化矩形变量为空（实际尺寸后续赋值）
	m_ViewRect.SetRectEmpty();
	m_FrameRect.SetRectEmpty();
	m_w = m_h = m_x2 = m_y2 = 0;

}

CDrawCADDoc::~CDrawCADDoc()
{
}

BOOL CDrawCADDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	//设置标题
	SetTitle(_T("画图"));

	//初始化变量

	m_StateShow.m_r = 1;
	m_StateShow.m_dx = 0;
	m_StateShow.m_dy = 0;
	m_StateShow.m_bViewPoint = true;
	m_Data.Clear();			//清空数据
	m_bSnapEnabled = TRUE;      // 默认开启吸附

	ClearUndoRedo();	// 清空历史

	return TRUE;
}




// CDrawCADDoc 序列化

void CDrawCADDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CDrawCADDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CDrawCADDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:     strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CDrawCADDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CDrawCADDoc 诊断

#ifdef _DEBUG
void CDrawCADDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDrawCADDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CDrawCADDoc 命令

//void CDrawCADDoc::OnFileNew()
//{
//	// TODO: 在此添加命令处理程序代码
//	if (!SaveModified())
//		return;  // 用户取消新建
//	CDocument::OnFileNew();  // 内部会调用 OnNewDocument()
//}

void CDrawCADDoc::OnFileSave()
{
	// TODO: 在此添加命令处理程序代码
		// 如果已有路径，直接保存，否则调用另存为
	if (m_strPathName.IsEmpty())
		OnFileSaveAs();
	else
	{
		// 根据扩展名决定保存格式
		CString ext = m_strPathName.Right(3).MakeLower();
		bool bSuccess = false;
		if (ext == _T("plt"))
			bSuccess = m_Data.SavePLT(m_strPathName);
		else if (ext == _T("dxf"))
			bSuccess = m_Data.SaveDXF(m_strPathName);
		else
		{
			AfxMessageBox(_T("无法识别文件格式，请使用“另存为”选择格式。"));
			OnFileSaveAs();
			return;
		}

		if (bSuccess)
		{
			SetModifiedFlag(FALSE);  // 清除修改标记
		}
	}
}

void CDrawCADDoc::OnFileSaveAs()
{
	// TODO: 在此添加命令处理程序代码
	TCHAR szFilter[] = _T("PLT 文件 (*.plt)|*.plt|DXF 文件 (*.dxf)|*.dxf|所有文件 (*.*)|*.*||");
	CFileDialog dlg(FALSE, _T("plt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, AfxGetMainWnd());

	if (IDOK == dlg.DoModal())
	{
		CString strPath = dlg.GetPathName();
		CString ext = dlg.GetFileExt().MakeLower();

		// 如果用户没有输入扩展名，根据选择的过滤器添加默认扩展名
		if (ext.IsEmpty())
		{
			int nFilterIndex = dlg.m_ofn.nFilterIndex;
			if (nFilterIndex == 1)
				strPath += _T(".plt");
			else if (nFilterIndex == 2)
				strPath += _T(".dxf");
			else
				strPath += _T(".plt"); // 默认
		}

		bool bSuccess = false;
		CString saveExt = strPath.Right(3).MakeLower();
		if (saveExt == _T("plt"))
			bSuccess = m_Data.SavePLT(strPath);
		else if (saveExt == _T("dxf"))
			bSuccess = m_Data.SaveDXF(strPath);
		else
		{
			AfxMessageBox(_T("不支持的文件扩展名！"));
			return;
		}
		if (bSuccess)
		{
			SetPathName(strPath);      // 更新文档路径
			SetModifiedFlag(FALSE);    // 清除修改标记
		}
	}
}

void CDrawCADDoc::OnFileOpen()
{
	// TODO: 在此添加命令处理程序代码
	TCHAR szFilter[] = _T("PLT 文件 (*.plt)|*.plt|DXF 文件 (*.dxf)|*.dxf|所有文件 (*.*)|*.*||");
	// 构造打开文件对话框
	CFileDialog dlg(TRUE, _T("plt"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilter, AfxGetMainWnd()); // 修改此行
	CString strFilePath;

	if (IDOK == dlg.DoModal())
	{
		strFilePath = dlg.GetPathName();
		CString ext = dlg.GetFileExt().MakeLower();

		bool bSuccess = false;
		if (ext == _T("plt"))
			bSuccess = m_Data.LoadPLT(strFilePath);
		else if (ext == _T("dxf"))
			bSuccess = m_Data.LoadDXF(strFilePath);
		else
		{
			AfxMessageBox(_T("不支持的文件格式！"));
			return;
		}
		if (bSuccess)
		{
			ClearUndoRedo();                 // 清空历史
			SetPathName(strFilePath);
			SetModifiedFlag(FALSE);

			// 设置文档路径并清除修改标记
			SetPathName(strFilePath);
			SetModifiedFlag(FALSE);
			// 加载成功后居中显示（可调用视图的适应函数）
			UpdateAllViews(NULL);
			// 如果需要自动居中，可以获取视图指针调用 CenterView()
			POSITION pos = GetFirstViewPosition();
			while (pos != NULL)
			{
				CView* pView = GetNextView(pos);
				if (pView->IsKindOf(RUNTIME_CLASS(CDrawCADView)))
				{
					CDrawCADView* pCADView = (CDrawCADView*)pView;
					pCADView->CenterView();
					break;
				}
			}
		}
	}
}

void CDrawCADDoc::OnInsertFile()
{
	// TODO: 在此添加命令处理程序代码
	TCHAR szFilter[] = _T("PLT 文件 (*.plt)|*.plt|DXF 文件 (*.dxf)|*.dxf|所有文件 (*.*)|*.*||");
	CFileDialog dlg(TRUE, _T("plt"), NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, szFilter, AfxGetMainWnd());
	if (IDOK != dlg.DoModal())
		return;

	CString strFilePath = dlg.GetPathName();
	CString ext = dlg.GetFileExt().MakeLower();

	// 先执行插入，不保存状态
	bool bSuccess = false;
	if (ext == _T("plt"))
		bSuccess = m_Data.InsertPLT(strFilePath);
	else if (ext == _T("dxf"))
		bSuccess = m_Data.InsertDXF(strFilePath);
	else
	{
		AfxMessageBox(_T("不支持的文件格式！"));
		return;
	}

	if (bSuccess)
	{
		SaveStateForUndo();               // 插入成功后才保存状态
		SetModifiedFlag(TRUE);
		UpdateAllViews(NULL);

		// 可选居中显示（代码不变）
		POSITION pos = GetFirstViewPosition();
		while (pos != NULL)
		{
			CView* pView = GetNextView(pos);
			if (pView->IsKindOf(RUNTIME_CLASS(CDrawCADView)))
			{
				CDrawCADView* pCADView = (CDrawCADView*)pView;
				// pCADView->CenterView();
				break;
			}
		}
	}
	else
	{
		AfxMessageBox(_T("插入文件失败！"));
		// 文档未改变，无需任何撤销栈操作
	}
}

//撤销
void CDrawCADDoc::SaveStateForUndo()
{
	m_undoStack.push_back(m_Data);
	m_redoStack.clear();  // 新操作清空重做栈
	if (m_undoStack.size() > MAX_UNDO_STEPS)
		m_undoStack.erase(m_undoStack.begin());
	SetModifiedFlag(TRUE);
}

void CDrawCADDoc::Undo()
{
	if (m_undoStack.empty()) return;
	m_redoStack.push_back(m_Data);
	m_Data = m_undoStack.back();
	m_undoStack.pop_back();
	UpdateAllViews(NULL);
}

void CDrawCADDoc::Redo()
{
	if (m_redoStack.empty()) return;
	m_undoStack.push_back(m_Data);
	m_Data = m_redoStack.back();
	m_redoStack.pop_back();
	UpdateAllViews(NULL);
}

void CDrawCADDoc::ClearUndoRedo()
{
	m_undoStack.clear();
	m_redoStack.clear();
}

void CDrawCADDoc::OnEditUndo()
{
	Undo();
}

void CDrawCADDoc::OnEditRedo()
{
	Redo();
}

void CDrawCADDoc::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_undoStack.empty());
}

void CDrawCADDoc::OnUpdateEditRedo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_redoStack.empty());
}


//复制粘贴
void CDrawCADDoc::OnEditCopy()
{
	m_Data.CopySelectedToClipboard();
}

void CDrawCADDoc::OnEditCut()
{
	SaveStateForUndo();                     // 保存当前状态，以便撤销
	m_Data.CutSelectedToClipboard();
	UpdateAllViews(NULL);
	SetModifiedFlag(TRUE);
}

//void CDrawCADDoc::OnEditPaste()
//{
//	m_Data.PasteFromClipboard();
//	UpdateAllViews(NULL);
//	SetModifiedFlag(TRUE);
//}

void CDrawCADDoc::OnEditSelectAll()
{
	m_Data.SelectAll();
	UpdateAllViews(NULL);
}

void CDrawCADDoc::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Data.GetSelectedCount() > 0);
}

void CDrawCADDoc::OnUpdateEditCut(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Data.GetSelectedCount() > 0);
}

//void CDrawCADDoc::OnUpdateEditPaste(CCmdUI* pCmdUI)
//{
//	pCmdUI->Enable(IsClipboardFormatAvailable(m_Data.GetClipboardFormat()));
//}

void CDrawCADDoc::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Data.GetLineSum() > 0);
}

void CDrawCADDoc::PasteFromClipboard(const SPoint* pTargetWorld)
{
	SaveStateForUndo();                     // 保存当前状态，以便撤销粘贴
	m_Data.PasteFromClipboard(pTargetWorld);
	UpdateAllViews(NULL);
	SetModifiedFlag(TRUE);
}

void CDrawCADDoc::DeselectAll()
{
	m_Data.ClearSelected();                     // 清除选中的线条索引
	if (m_Data.GetCurLine())
	{
		m_Data.GetCurLine()->ClearCurPoint();    // 清除当前节点
	}
	m_Data.EndLine();                            // 将 m_CurLine 设为 nullptr
}

BOOL CDrawCADDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	// 先调用基类，基类会调用 DeleteContents 清空文档，并设置路径等
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	// 获取文件扩展名（小写）
	CString strPath = lpszPathName;
	CString ext = strPath.Right(3).MakeLower();  // 取最后3个字符

	bool bSuccess = false;
	if (ext == _T("plt"))
		bSuccess = m_Data.LoadPLT(strPath);
	else if (ext == _T("dxf"))
		bSuccess = m_Data.LoadDXF(strPath);
	else
	{
		AfxMessageBox(_T("不支持的文件格式！"));
		return FALSE;
	}

	if (!bSuccess)
	{
		AfxMessageBox(_T("加载文件失败！"));
		return FALSE;
	}

	// 设置文档修改标记为未修改（刚加载的文件）
	SetModifiedFlag(FALSE);

	// 加载后居中显示（可调用视图的 CenterView 方法）
	POSITION pos = GetFirstViewPosition();
	while (pos != NULL)
	{
		CView* pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(CDrawCADView)))
		{
			CDrawCADView* pCADView = (CDrawCADView*)pView;
			pCADView->CenterView();   // 居中显示（你需要确保 CenterView 存在）
			break;
		}
	}

	return TRUE;
}
