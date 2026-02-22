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

// MainFrm.cpp: CMainFrame 类的实现
//

#include "pch.h"
#include "framework.h"
#include "DrawCAD.h"

#include "MainFrm.h"
#include "DrawCADDoc.h"
#include "DrawCADView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_PRINT, &CMainFrame::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CMainFrame::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnFilePrintPreview)
	ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnUpdateFilePrintPreview)
	//combo
	//1.
	ON_COMMAND(ID_SHOWPOINT, &CMainFrame::OnNodeComboSelChanged)
	ON_UPDATE_COMMAND_UI(ID_SHOWPOINT, &CMainFrame::OnUpdateNodeCombo)
	//2.
	ON_COMMAND(ID_MODE, &CMainFrame::OnModeComboSelChanged)
	ON_UPDATE_COMMAND_UI(ID_MODE, &CMainFrame::OnUpdateModeCombo)
	//3.
	ON_COMMAND(ID_SCALNUM, &CMainFrame::OnScalNumComboSelChanged)
	ON_UPDATE_COMMAND_UI(ID_SCALNUM, &CMainFrame::OnUpdateScalNumCombo)
	//4.
	ON_COMMAND(ID_MOVESTEP, &CMainFrame::OnMoveComboSelChanged)
	ON_UPDATE_COMMAND_UI(ID_MOVESTEP, &CMainFrame::OnUpdateMoveCombo)
	//5.
	ON_COMMAND(ID_LINEWIDTH, &CMainFrame::OnLineWidthComboSelChanged)
	ON_UPDATE_COMMAND_UI(ID_LINEWIDTH, &CMainFrame::OnUpdateLineWidthCombo)
	//6.
	ON_COMMAND(ID_LINETYPE, &CMainFrame::OnLineTypeComboSelChanged)
	ON_UPDATE_COMMAND_UI(ID_LINETYPE, &CMainFrame::OnUpdateLineTypeCombo)
	//7.
	ON_COMMAND(ID_SNAPSCAL, &CMainFrame::OnSnapScalComboSelChanged)
	ON_UPDATE_COMMAND_UI(ID_SNAPSCAL, &CMainFrame::OnUpdateSnapScalCombo)

	ON_COMMAND(ID_ZOOMIN, &CMainFrame::OnZoomin)
	ON_COMMAND(ID_ZOOMOUT, &CMainFrame::OnZoomout)
	ON_COMMAND(ID_ZOOMINLAST, &CMainFrame::OnZoominlast)
	ON_COMMAND(ID_ZOOMOUTLAST, &CMainFrame::OnZoomoutlast)
	ON_COMMAND(ID_MOVEUP, &CMainFrame::OnMoveup)
	ON_COMMAND(ID_MOVEDOWN, &CMainFrame::OnMovedown)
	ON_COMMAND(ID_MOVELEFT, &CMainFrame::OnMoveleft)
	ON_COMMAND(ID_MOVERIGHT, &CMainFrame::OnMoveright)
	ON_COMMAND(ID_SNAP, &CMainFrame::OnSnap)
	ON_UPDATE_COMMAND_UI(ID_SNAP, &CMainFrame::OnUpdateSnap)
END_MESSAGE_MAP()

// CMainFrame 构造/析构

CMainFrame::CMainFrame() noexcept
{
	// TODO: 在此添加成员初始化代码
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}

	CString strTitlePane1;
	CString strTitlePane2;
	bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
	ASSERT(bNameValid);
	bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
	ASSERT(bNameValid);
	//m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, strTitlePane1, TRUE), strTitlePane1);
	//m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, strTitlePane2, TRUE), strTitlePane2);
	
	// 添加窗格，使用稍长的初始文本以确保有足够宽度
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, _T("图形总数: 0 ; 缩放倍数: 1.00; 横向偏移:0.00 ; 纵向偏移: 0.00 ; 文件路径 :000000000000000000000000000000000000000000000000000000"), TRUE), strTitlePane1);
	m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, _T("鼠标世界坐标: (0.000000, 0.000000)"), TRUE), strTitlePane2);

	// 启用 Visual Studio 2005 样式停靠窗口行为
	CDockingManager::SetDockingMode(DT_SMART);
	// 启用 Visual Studio 2005 样式停靠窗口自动隐藏行为
	EnableAutoHidePanes(CBRS_ALIGN_ANY);
	
	//设置图标
	SetClassLongPtr(m_hWnd, GCLP_HICON, (LONG_PTR)AfxGetApp()->LoadIcon(IDI_WIN));

	//设置标题
	SetTitle(TEXT("DrawCAD"));

	//居中显示
	CenterWindow();

	//添加combo
	//1.
	CMFCRibbonBaseElement* pElem = m_wndRibbonBar.FindByID(ID_SHOWPOINT);
	if (pElem != nullptr && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonComboBox)))
	{
		// 2. 转换类型并赋值给成员变量
		m_ShowPointCombo = (CMFCRibbonComboBox*)pElem;

		m_ShowPointCombo->AddItem(_T("显示节点"));
		m_ShowPointCombo->AddItem(_T("隐藏节点"));
		m_ShowPointCombo->SelectItem(0); // 默认选中第一个
	}
	else
	{
		TRACE0("未找到节点下拉框！\n");
		m_ShowPointCombo = nullptr; // 找不到就置空
	}
	//2.
	pElem = m_wndRibbonBar.FindByID(ID_MODE);
	if (pElem != nullptr && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonComboBox)))
	{
		// 2. 转换类型并赋值给成员变量
		m_ModeCombo = (CMFCRibbonComboBox*)pElem;

		m_ModeCombo->AddItem(_T("绘图"));
		m_ModeCombo->AddItem(_T("选择"));
		m_ModeCombo->SelectItem(0); // 默认选中第一个
	}
	else
	{
		TRACE0("未找到节点下拉框！\n");
		m_ModeCombo = nullptr; // 找不到就置空
	}
	//3.
	pElem = m_wndRibbonBar.FindByID(ID_SCALNUM);
	if (pElem != nullptr && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonComboBox)))
	{
		// 2. 转换类型并赋值给成员变量
		m_ScalNumCombo = (CMFCRibbonComboBox*)pElem;
		m_ScalNumCombo->AddItem(_T("10%"));
		m_ScalNumCombo->AddItem(_T("50%"));
		m_ScalNumCombo->AddItem(_T("100%"));
		m_ScalNumCombo->AddItem(_T("120%"));
		m_ScalNumCombo->AddItem(_T("160%"));
		m_ScalNumCombo->AddItem(_T("200%"));
		m_ScalNumCombo->AddItem(_T("400%"));
		m_ScalNumCombo->AddItem(_T("700%"));
		m_ScalNumCombo->AddItem(_T("1000%"));
		m_ScalNumCombo->SelectItem(2); // 默认选中100%
	}
	else
	{
		TRACE0("未找到节点下拉框！\n");
		m_ScalNumCombo = nullptr; // 找不到就置空
	}
	//4.
	pElem = m_wndRibbonBar.FindByID(ID_MOVESTEP);
	if (pElem != nullptr && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonComboBox)))
	{
		// 2. 转换类型并赋值给成员变量
		m_MoveCombo = (CMFCRibbonComboBox*)pElem;
		m_MoveCombo->AddItem(_T("0.1mm"));
		m_MoveCombo->AddItem(_T("1.0mm"));
		m_MoveCombo->AddItem(_T("1.2mm"));
		m_MoveCombo->AddItem(_T("1.6mm"));
		m_MoveCombo->AddItem(_T("2.0mm"));
		m_MoveCombo->AddItem(_T("4.0mm"));
		m_MoveCombo->AddItem(_T("7.0mm"));
		m_MoveCombo->AddItem(_T("10.0mm"));
		m_MoveCombo->AddItem(_T("20.0mm"));
		m_MoveCombo->SelectItem(2); // 默认选中第三个
	}
	else
	{
		TRACE0("未找到节点下拉框！\n");
		m_MoveCombo = nullptr; // 找不到就置空
	}
	//5.
	pElem = m_wndRibbonBar.FindByID(ID_LINEWIDTH);
	if (pElem != nullptr && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonComboBox)))
	{
		// 2. 转换类型并赋值给成员变量
		m_LineWidthCombo = (CMFCRibbonComboBox*)pElem;
		m_LineWidthCombo->AddItem(_T("1mm"));
		m_LineWidthCombo->AddItem(_T("2mm"));
		m_LineWidthCombo->AddItem(_T("4mm"));
		m_LineWidthCombo->AddItem(_T("7mm"));
		m_LineWidthCombo->AddItem(_T("10mm"));
		m_LineWidthCombo->AddItem(_T("20mm"));
		m_LineWidthCombo->SelectItem(0);
	}
	else
	{
		TRACE0("未找到节点下拉框！\n");
		m_LineWidthCombo = nullptr; // 找不到就置空
	}
	//6.
	pElem = m_wndRibbonBar.FindByID(ID_LINETYPE);
	if (pElem != nullptr && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonComboBox)))
	{
		// 2. 转换类型并赋值给成员变量
		m_LineTypeCombo = (CMFCRibbonComboBox*)pElem;
		m_LineTypeCombo->AddItem(_T("实线"));
		m_LineTypeCombo->AddItem(_T("虚线"));
		m_LineTypeCombo->AddItem(_T("点线"));
		m_LineTypeCombo->SelectItem(0);
	}
	else
	{
		TRACE0("未找到节点下拉框！\n");
		m_LineTypeCombo = nullptr; // 找不到就置空
	}
	//7.
	pElem = m_wndRibbonBar.FindByID(ID_SNAPSCAL);
	if (pElem != nullptr && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonComboBox)))
	{
		// 2. 转换类型并赋值给成员变量
		m_SnapSaclCombo = (CMFCRibbonComboBox*)pElem;
		m_SnapSaclCombo->AddItem(_T("5"));
		m_SnapSaclCombo->AddItem(_T("10"));
		m_SnapSaclCombo->AddItem(_T("20"));
		m_SnapSaclCombo->AddItem(_T("30"));
		m_SnapSaclCombo->AddItem(_T("40"));
		m_SnapSaclCombo->SelectItem(0);
	}
	else
	{
		TRACE0("未找到节点下拉框！\n");
		m_SnapSaclCombo = nullptr; // 找不到就置空
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序


void CMainFrame::OnFilePrint()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_PRINT);
	}
}

void CMainFrame::OnFilePrintPreview()
{
	if (IsPrintPreview())
	{
		PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);  // 强制关闭“打印预览”模式
	}
}

void CMainFrame::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(IsPrintPreview());
}

void CMainFrame::GetFrameSize()
{
	// 获取活动视图
	// 确保窗口已创建完成
	if (!IsWindow(m_hWnd)) return;

	CView* pView = GetActiveView();
	if (pView == nullptr) return;
	CDrawCADDoc* pDoc = static_cast<CDrawCADDoc*>(pView->GetDocument());
	if (pDoc == nullptr) return;
	// 1. 整体窗口大小
	/*CRect rectWindow;
	GetWindowRect(pDoc->m_FrameRect);*/
	// 2. 客户区大小
	CRect rectClient;
	GetClientRect(pDoc->m_FrameRect);

}


void CMainFrame::OnNodeComboSelChanged()
{
	if (m_ShowPointCombo == nullptr) return;
	int Index = m_ShowPointCombo->GetCurSel();

	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView == nullptr) return;
	CDrawCADDoc* pDoc = pView->GetDocument();
	if (pDoc == nullptr) return;
	
	if (Index == 0)
	{
		pDoc->m_StateShow.m_bViewPoint = true;
		Invalidate();
	}
	else if (Index == 1)
	{
		pDoc->m_StateShow.m_bViewPoint = false;
		Invalidate();
	}
}
void CMainFrame::OnUpdateNodeCombo(CCmdUI* pCmdUI)
{
	// 控制下拉框是否可用（示例：始终可用）
	pCmdUI->Enable(true);
}

void CMainFrame::OnModeComboSelChanged()
{
	// 先检查成员变量是否有效
	if (m_ModeCombo == nullptr) return;
	int Index = m_ModeCombo->GetCurSel();

	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView == nullptr) return;
	CDrawCADDoc* pDoc = pView->GetDocument();
	if (pDoc == nullptr) return;

	if (Index == 0)
	{
		pDoc->DrawMod = LineMode;
	}
	else if (Index == 1)
	{
		pDoc->DrawMod = SelectMode;
		pView->ClearRubberBand();   // 清除橡皮筋
		pView->Invalidate();				// 清除多余橡皮筋
	}
	pView->ClearHighlight();
	pView->Invalidate();
}
void CMainFrame::OnUpdateModeCombo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
}

void CMainFrame::OnScalNumComboSelChanged()			//选择缩放倍率
{
	if (m_ScalNumCombo == nullptr) return;

	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView == nullptr) return;
	CDrawCADDoc* pDoc = pView->GetDocument();
	if (pDoc == nullptr) return;

	CString strText = m_ScalNumCombo->GetEditText();
	strText.Trim();

	// 1. 提取数字部分（忽略前缀，捕获数字、小数点、负号）
	CString strNum;
	int nPos = 0;
	while (nPos < strText.GetLength() &&
		!_istdigit(strText[nPos]) &&
		strText[nPos] != _T('.') &&
		strText[nPos] != _T('-'))
		nPos++;
	while (nPos < strText.GetLength())
	{
		TCHAR ch = strText[nPos];
		if (_istdigit(ch) || ch == _T('.') || ch == _T('-'))
		{
			strNum += ch;
			nPos++;
		}
		else
		{
			break;
		}
	}

	if (strNum.IsEmpty())
	{
		RestoreScaleNumDisplay(pDoc);
		AfxMessageBox(_T("请输入有效的百分比数字！"));
		return;
	}

	double dPercent = _ttof(strNum);  // 用户输入的百分比数值
	if (dPercent <= 0.0 || dPercent > MAX_SCALE * 100.0)
	{
		RestoreScaleNumDisplay(pDoc);
		AfxMessageBox(_T("请输入大于0的有效百分比！"));
		return;
	}

	// 2. 转换为缩放倍率（除以100）
	double dScale = dPercent / 100.0;
	if (dScale > MAX_SCALE)
	{
		RestoreScaleNumDisplay(pDoc);
		AfxMessageBox(_T("缩放倍数过大，已自动限制到最大值。"));
		dScale = MAX_SCALE;
	}

	// 3. 保存旧的缩放和平移，计算新的平移量（保持视图中心不变）
	double old_r = pDoc->m_StateShow.m_r;
	double old_dx = pDoc->m_StateShow.m_dx;
	double old_dy = pDoc->m_StateShow.m_dy;

	double viewCx = pDoc->m_w / 2.0;
	double viewCy = pDoc->m_h / 2.0;

	double worldX = (viewCx - old_dx) / old_r;
	double worldY = -(viewCy + old_dy) / old_r;

	pDoc->m_StateShow.m_r = dScale;
	pDoc->m_StateShow.m_dx = viewCx - worldX * dScale;
	pDoc->m_StateShow.m_dy = -viewCy - worldY * dScale;

	// 4. 自动补全 "%" 后缀
	CString strSuffix = _T("%");
	if (strText.Right(1).CompareNoCase(strSuffix) != 0)
	{
		CString newText;
		newText.Format(_T("%g"), dPercent);
		newText += strSuffix;
		m_ScalNumCombo->SetEditText(newText);
	}

	Invalidate();
}
void CMainFrame::OnUpdateScalNumCombo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
}
void CMainFrame::UpdateScaleCombo()			//更新缩放倍率显示
{
	// TODO: 在此处添加实现代码.
	if (m_ScalNumCombo == nullptr) return;

	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView == nullptr) return;

	CDrawCADDoc* pDoc = pView->GetDocument();
	if (pDoc == nullptr) return;

	double scale = pDoc->m_StateShow.m_r;
	double percent = scale * 100.0;

	CString strPercent;
	// 格式化为百分比，保留一位小数
	strPercent.Format(_T("%.0f%%"), percent);

	m_ScalNumCombo->SetEditText(strPercent);

}
void CMainFrame::RestoreScaleNumDisplay(CDrawCADDoc* pDoc)
{
	double scale = pDoc->m_StateShow.m_r;
	double percent = scale * 100.0;
	CString strPercent;
	strPercent.Format(_T("%.1f%%"), percent); // 与 UpdateScaleCombo 一致
	m_ScalNumCombo->SetEditText(strPercent);
}

void CMainFrame::OnMoveComboSelChanged()
{
	if (m_MoveCombo == nullptr) return;

	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView == nullptr) return;
	CDrawCADDoc* pDoc = pView->GetDocument();
	if (pDoc == nullptr) return;

	CString strText = m_MoveCombo->GetEditText();
	strText.Trim();

	// 1. 提取数字部分（跳过前缀，捕获数字、小数点、负号）
	CString strNum;
	int nPos = 0;
	// 寻找数字开始位置
	while (nPos < strText.GetLength() &&
		!_istdigit(strText[nPos]) &&
		strText[nPos] != _T('.') &&
		strText[nPos] != _T('-'))
		nPos++;
	// 从该位置开始收集连续的数字/小数点/负号
	while (nPos < strText.GetLength())
	{
		TCHAR ch = strText[nPos];
		if (_istdigit(ch) || ch == _T('.') || ch == _T('-'))
		{
			strNum += ch;
			nPos++;
		}
		else
		{
			break;
		}
	}

	if (strNum.IsEmpty())
	{
		// 恢复为当前有效值
		RestoreMoveStepDisplay(pDoc);
		AfxMessageBox(_T("请输入有效的数字！"));
		return;
	}

	double dResult = _ttof(strNum);
	// 检查范围（根据您的需求调整）
	if (dResult <= 0 || dResult > 500)
	{
		// 恢复为当前有效值
		RestoreMoveStepDisplay(pDoc);
		AfxMessageBox(_T("请输入大于0且小于500的有效数字！"));
		return;
	}

	// 2. 更新文档的步进值（单位为毫米）
	pDoc->m_MoveStep = dResult;

	// 3. 若编辑框文本不以 "mm" 结尾，则补上 "mm" 并更新显示
	CString strSuffix = _T("mm");
	if (strText.Right(2).CompareNoCase(strSuffix) != 0)
	{
		CString newText;
		newText.Format(_T("%g"), dResult);
		newText += strSuffix;
		m_MoveCombo->SetEditText(newText);
	}
}
void CMainFrame::OnUpdateMoveCombo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
}
void CMainFrame::RestoreMoveStepDisplay(CDrawCADDoc* pDoc)// 恢复移动步进显示
{
	CString strDisplay;
	strDisplay.Format(_T("%gmm"), pDoc->m_MoveStep);
	m_MoveCombo->SetEditText(strDisplay);
}

void CMainFrame::OnLineWidthComboSelChanged()
{
	if (m_LineWidthCombo == nullptr) return;

	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView == nullptr) return;
	CDrawCADDoc* pDoc = pView->GetDocument();
	if (pDoc == nullptr) return;

	CString strText = m_LineWidthCombo->GetEditText();
	strText.Trim();

	// 提取数字部分（跳过非数字前缀）
	CString strNum;
	int nPos = 0;
	while (nPos < strText.GetLength() &&
		!_istdigit(strText[nPos]) &&
		strText[nPos] != _T('.') &&
		strText[nPos] != _T('-'))
		nPos++;
	while (nPos < strText.GetLength())
	{
		TCHAR ch = strText[nPos];
		if (_istdigit(ch) || ch == _T('.') || ch == _T('-'))
		{
			strNum += ch;
			nPos++;
		}
		else
		{
			break;
		}
	}

	if (strNum.IsEmpty())
	{
		RestoreLineWidthDisplay(pDoc);
		AfxMessageBox(_T("请输入有效的数字！"));
		return;
	}

	double dValue = _ttof(strNum); // 或 _tcstod(strNum, NULL)
	if (dValue <= 0.0 || dValue > 20.0)
	{
		RestoreLineWidthDisplay(pDoc);
		AfxMessageBox(_T("请输入大于0且小于20的有效数字！"));
		return;
	}

	// 四舍五入取整（线宽为整数）
	int iResult = static_cast<int>(dValue + 0.5);

	// 更新文档线宽
	pDoc->m_LineWidth = iResult;
	Invalidate();

	// 自动补全 "mm" 后缀
	CString strSuffix = _T("mm");
	if (strText.Right(2).CompareNoCase(strSuffix) != 0)
	{
		CString newText;
		newText.Format(_T("%d"), iResult);
		newText += strSuffix;
		m_LineWidthCombo->SetEditText(newText);
	}
}
void CMainFrame::OnUpdateLineWidthCombo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
}
void CMainFrame::RestoreLineWidthDisplay(CDrawCADDoc* pDoc)
{
	CString strDisplay;
	strDisplay.Format(_T("%dmm"), pDoc->m_LineWidth);
	m_LineWidthCombo->SetEditText(strDisplay);
}

void CMainFrame::OnLineTypeComboSelChanged()
{
	// 先检查成员变量是否有效
	if (m_LineTypeCombo == nullptr) return;

	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView == nullptr) return;
	CDrawCADDoc* pDoc = pView->GetDocument();
	if (pDoc == nullptr) return;

	int dResult = m_LineTypeCombo->GetCurSel();
	pDoc->m_LineStyle = dResult;
	Invalidate();
}
void CMainFrame::OnUpdateLineTypeCombo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
}

void CMainFrame::OnSnapScalComboSelChanged()
{
	// 先检查成员变量是否有效
	if (m_SnapSaclCombo == nullptr) return;

	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView == nullptr) return;
	CDrawCADDoc* pDoc = pView->GetDocument();
	if (pDoc == nullptr) return;

	CString strText = m_SnapSaclCombo->GetEditText();
	int dResult = 0;
	TCHAR* pEnd; // 指向转换结束位置的指针
	dResult = _tcstod(strText, &pEnd);
	if (pEnd == strText || *pEnd != '\0' || dResult <= 0 || dResult > 50)
	{
		AfxMessageBox(_T("请输入大于0小于50的有效的数字（如10）！"));
		return;
	}
	pView->SNAP_TOLERANCE = dResult;
	Invalidate();
}
void CMainFrame::OnUpdateSnapScalCombo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
}

void CMainFrame::OnZoomin()  			//放大	
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView) {
		CDrawCADDoc* pDoc = pView->GetDocument();
		if (!pDoc) return;					// 文档指针无效则退出
		double temp = pDoc->m_StateShow.m_r * pDoc->m_nZoom;
		if (temp > MAX_SCALE) temp = MAX_SCALE;   // 限制上限
		pDoc->m_StateShow.m_r = temp;
		UpdateScaleCombo();//更新显示倍率
		//延中心放大
		pView->CenterView();
	}
}

void CMainFrame::OnZoomout()			//缩小
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView) {
		CDrawCADDoc* pDoc = pView->GetDocument();
		if (!pDoc) return;					// 文档指针无效则退出
		double temp = pDoc->m_StateShow.m_r / pDoc->m_nZoom;
		if (temp < MIN_SCALE) temp = MIN_SCALE;   // 限制下限
		pDoc->m_StateShow.m_r = temp;
		UpdateScaleCombo();//更新显示倍率
		//延中心缩小	
		pView->CenterView();
	}
}

void CMainFrame::OnZoominlast()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (!pView) return;

	CDrawCADDoc* pDoc = pView->GetDocument();
	if (!pDoc) return;

	CPoint zoomPt;
	if (pView->HasLastZoomPoint()){
		zoomPt = pView->GetLastZoomPoint();
		pView->ZoomAtPoint(zoomPt, pDoc->m_nZoom);
	}
	else
	{
		
		OnZoomin();			// 如果没有保存的点，则延视图中心
	}
}

void CMainFrame::OnZoomoutlast()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (!pView) return;

	CDrawCADDoc* pDoc = pView->GetDocument();
	if (!pDoc) return;

	CPoint zoomPt;
	if (pView->HasLastZoomPoint()){
		zoomPt = pView->GetLastZoomPoint();
		pView->ZoomAtPoint(zoomPt, 1.0 / pDoc->m_nZoom);
	}
	else
	{
		OnZoomout();		// 如果没有保存的点，则延视图中心
	}
}

void CMainFrame::SetStatusBarPaneText(UINT nID, const CString& strText)
{
	// TODO: 在此处添加实现代码.
	CMFCRibbonBaseElement* pElem = m_wndStatusBar.FindByID(nID);// 通过 ID 查找窗格元素
	if (pElem && pElem->IsKindOf(RUNTIME_CLASS(CMFCRibbonStatusBarPane)))
	{
		CMFCRibbonStatusBarPane* pPane = (CMFCRibbonStatusBarPane*)pElem;
		pPane->SetText(strText);
		// 可选：立即刷新状态栏
		m_wndStatusBar.RedrawWindow();
	}
}

void CMainFrame::OnMoveup()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (!pView) return;

	CDrawCADDoc* pDoc = pView->GetDocument();
	if (!pDoc) return;

	double step = pDoc->m_MoveStep;   // 使用文档中设定的步长（世界单位）
	pView->MoveByStep(0.0, step);     // 向上移动：Y 增加
}

void CMainFrame::OnMovedown()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (!pView) return;

	CDrawCADDoc* pDoc = pView->GetDocument();
	if (!pDoc) return;

	double step = pDoc->m_MoveStep;
	pView->MoveByStep(0.0, -step);    // 向下移动：Y 减少
}

void CMainFrame::OnMoveleft()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (!pView) return;

	CDrawCADDoc* pDoc = pView->GetDocument();
	if (!pDoc) return;

	double step = pDoc->m_MoveStep;
	pView->MoveByStep(-step, 0.0);    // 向左移动：X 减少
}

void CMainFrame::OnMoveright()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (!pView) return;

	CDrawCADDoc* pDoc = pView->GetDocument();
	if (!pDoc) return;

	double step = pDoc->m_MoveStep;
	pView->MoveByStep(step, 0.0);     // 向右移动：X 增加
}

void CMainFrame::OnSnap()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (!pView) return;

	CDrawCADDoc* pDoc = pView->GetDocument();
	if (!pDoc) return;

	// 切换吸附状态
	pDoc->m_bSnapEnabled = !pDoc->m_bSnapEnabled;

	// 如果关闭吸附，清除视图中的高亮
	if (!pDoc->m_bSnapEnabled)
	{
		pView->ClearHighlight();
	}

	// 刷新视图
	pView->Invalidate();
}

void CMainFrame::OnUpdateSnap(CCmdUI* pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	CDrawCADView* pView = dynamic_cast<CDrawCADView*>(GetActiveView());
	if (pView && pView->GetDocument())
	{
		pCmdUI->SetCheck(pView->GetDocument()->m_bSnapEnabled ? 1 : 0);
	}
	else
	{
		pCmdUI->Enable(FALSE);
	}
}

void CMainFrame::SetModeComboSelection(int nIndex)
{
	if (m_ModeCombo != nullptr)
	{
		m_ModeCombo->SelectItem(nIndex);   // 0 = 绘图, 1 = 选择
	}
}