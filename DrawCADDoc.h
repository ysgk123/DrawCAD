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

// DrawCADDoc.h: CDrawCADDoc 类的接口
//


#pragma once
#include "m_CData.h"
#include <vector>
#include <afxole.h>         // 剪贴板操作

class CDrawCADDoc : public CDocument
{
protected: // 仅从序列化创建
	CDrawCADDoc() noexcept;
	DECLARE_DYNCREATE(CDrawCADDoc)

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CDrawCADDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	m_CData m_Data;							// 负责存储信息

	//CFont m_Font;

	SPoint m_Sorigin, m_Worigin;			// 屏幕原点 和 窗口原点 默认0,0

	COLORREF m_LineColor;					// 画线颜色
	COLORREF m_CurLineColor;				// 当前画线颜色
	COLORREF m_BackColor;					// 当前画布颜色
	int m_LineWidth;						// 画线宽度
	int m_LineStyle;						// 画线样式0实线 1虚线 2点线

	CRect m_ViewRect;						// 绘图矩形
	CRect m_FrameRect;						// 绘图矩形
	SState m_StateShow;						// 显示状态
	double m_MoveStep;						// 移动步长
	double m_nZoom;							// 缩放比率
	int m_w, m_h;							// 绘图窗口宽高
	int m_x2, m_y2;							// 框架窗口宽高
	DrawingMode DrawMod;					// 绘图模式
	BOOL m_bSnapEnabled;					// TRUE启用吸附

	//afx_msg void OnFileNew();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileOpen();
	afx_msg void OnInsertFile();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

public:
	// 撤销/重做
	void SaveStateForUndo();
	void Undo();
	void Redo();
	void ClearUndoRedo();
	

protected:
	std::vector<m_CData> m_undoStack;
	std::vector<m_CData> m_redoStack;
	// 消息处理
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);

//复制粘贴
public:
	afx_msg void OnEditCopy();
	afx_msg void OnEditCut();
//	afx_msg void OnEditPaste();
	afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
//	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	void PasteFromClipboard(const SPoint* pTargetWorld = nullptr);
	void DeselectAll();   // 取消所有选择
	
};
