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

// MainFrm.h: CMainFrame 类的接口
//

#include "m_CData.h"
#include "DrawCADDoc.h"

#pragma once

class CMainFrame : public CFrameWndEx
{
	
protected: // 仅从序列化创建
	CMainFrame() noexcept;
	DECLARE_DYNCREATE(CMainFrame)

// 特性
public:

// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 控件条嵌入成员
	CMFCRibbonBar     m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar  m_wndStatusBar;

// 生成的消息映射函数
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

public:
	void GetFrameSize();

protected:
	// 自定义消息处理函数 添加combo
	//1.
	CMFCRibbonComboBox* m_ShowPointCombo; // 节点下拉框指针
	// 消息处理函数声明（后续处理下拉框选择事件）
	afx_msg void OnNodeComboSelChanged();
	afx_msg void OnUpdateNodeCombo(CCmdUI* pCmdUI);
	//2.
	CMFCRibbonComboBox* m_ModeCombo; // 节点下拉框指针
	// 消息处理函数声明（后续处理下拉框选择事件）
	afx_msg void OnModeComboSelChanged();
	afx_msg void OnUpdateModeCombo(CCmdUI* pCmdUI);
	//3.
	CMFCRibbonComboBox* m_ScalNumCombo; // 节点下拉框指针
	// 消息处理函数声明（后续处理下拉框选择事件）
	afx_msg void OnScalNumComboSelChanged();
	afx_msg void OnUpdateScalNumCombo(CCmdUI* pCmdUI);
	void RestoreScaleNumDisplay(CDrawCADDoc* pDoc);
	//4.
	CMFCRibbonComboBox* m_MoveCombo; // 节点下拉框指针
	// 消息处理函数声明（后续处理下拉框选择事件）
	afx_msg void OnMoveComboSelChanged();
	afx_msg void OnUpdateMoveCombo(CCmdUI* pCmdUI);
	void RestoreMoveStepDisplay(CDrawCADDoc* pDoc);	
	//5.
	CMFCRibbonComboBox* m_LineWidthCombo; // 节点下拉框指针
	// 消息处理函数声明（后续处理下拉框选择事件）
	afx_msg void OnLineWidthComboSelChanged();
	afx_msg void OnUpdateLineWidthCombo(CCmdUI* pCmdUI);
	void RestoreLineWidthDisplay(CDrawCADDoc* pDoc);
	//6.
	CMFCRibbonComboBox* m_LineTypeCombo; // 节点下拉框指针
	// 消息处理函数声明（后续处理下拉框选择事件）
	afx_msg void OnLineTypeComboSelChanged();
	afx_msg void OnUpdateLineTypeCombo(CCmdUI* pCmdUI);
	//7.
	CMFCRibbonComboBox* m_SnapSaclCombo; // 节点下拉框指针
	// 消息处理函数声明（后续处理下拉框选择事件）
	afx_msg void OnSnapScalComboSelChanged();
	afx_msg void OnUpdateSnapScalCombo(CCmdUI* pCmdUI);
public:
	void UpdateScaleCombo();

	afx_msg void OnZoomin();
	afx_msg void OnZoomout();
	
	afx_msg void OnZoominlast();
	afx_msg void OnZoomoutlast();

	void SetStatusBarPaneText(UINT nID, const CString& strText);
	afx_msg void OnMoveup();
	afx_msg void OnMovedown();
	afx_msg void OnMoveleft();
	afx_msg void OnMoveright();
	afx_msg void OnSnap();
	afx_msg void OnUpdateSnap(CCmdUI* pCmdUI);

	void SetModeComboSelection(int nIndex);   // 设置模式组合框的选中项
};


