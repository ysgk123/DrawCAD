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

// DrawCADView.cpp: CDrawCADView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "DrawCAD.h"
#endif

#include "DrawCADDoc.h"
#include "DrawCADView.h"
#include "MainFrm.h"
#include "m_CData.h"
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const SPoint CDrawCADView::s_angleDirs[8] = {
	{ 1.0, 0.0 },									// 0°
	{ 0.7071067811865476, 0.7071067811865476 },		// 45°
	{ 0.0, 1.0 },									// 90°
	{ -0.7071067811865476, 0.7071067811865476 },	// 135°
	{ -1.0, 0.0 },									// 180°
	{ -0.7071067811865476, -0.7071067811865476 },	// 225°
	{ 0.0, -1.0 },									// 270°
	{ 0.7071067811865476, -0.7071067811865476 }		// 315°
};

// CDrawCADView

IMPLEMENT_DYNCREATE(CDrawCADView, CView)

BEGIN_MESSAGE_MAP(CDrawCADView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CDrawCADView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_LINECOLOR, &CDrawCADView::OnLinecolor)
	ON_COMMAND(ID_CURLINECOLOR, &CDrawCADView::OnCurlinecolor)
	ON_COMMAND(ID_BACKCOLOR, &CDrawCADView::OnBackcolor)
	ON_COMMAND(ID_DELPOINT, &CDrawCADView::OnDelpoint)
	ON_COMMAND(ID_DELLINE, &CDrawCADView::OnDelline)
	ON_COMMAND(ID_ADAPT, &CDrawCADView::OnAdapt)
	ON_WM_MOUSEWHEEL()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_EDIT_PASTE, &CDrawCADView::OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &CDrawCADView::OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_DELLINE, &CDrawCADView::OnUpdateDelline)
	ON_UPDATE_COMMAND_UI(ID_DELPOINT, &CDrawCADView::OnUpdateDelpoint)
	ON_WM_KEYDOWN()	//esc
	ON_WM_KEYUP()
END_MESSAGE_MAP()

// CDrawCADView 构造/析构

CDrawCADView::CDrawCADView() noexcept 
	: m_bDragging(false)
	, m_dragStart(0, 0)
	, m_dragStartDx(0.0)
	, m_dragStartDy(0.0)
	, m_lastZoomPoint(0, 0)
	, m_bHasZoomPoint(false)
	, m_dragMode(None)
	, m_dragLineIdx(-1)
	, m_dragPointIdx(-1)
	, m_bRubberBanding(false)          
	, m_rubberStartWorld(0, 0, 0)       
	, m_rubberEndWorld(0, 0, 0)
	, m_dragStartMouseWorld(0, 0, 0)
	, m_bHighlightPoint(false)
	, m_highlightLineIdx(-1)
	, m_highlightPointIdx(-1)
	, m_bAngleSnapEnabled(false)
	, m_bestAngleDirIndex(-1)
	, m_bestAngleDirPositive(true)
{
	// TODO: 在此处添加构造代码


}

CDrawCADView::~CDrawCADView()
{
}

BOOL CDrawCADView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CDrawCADView 绘图

void CDrawCADView::OnDraw(CDC* pDC)
{
	//// TODO: 在此处为本机数据添加绘制代码
	//Draw(pDC);

	CDrawCADDoc* pDoc = GetDocument();	//双缓冲
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	CRect rectClient;
	GetClientRect(rectClient);

	// 创建与屏幕 DC 兼容的内存 DC
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	// 创建与屏幕 DC 兼容的位图，大小为客户区尺寸
	CBitmap memBitmap;
	memBitmap.CreateCompatibleBitmap(pDC, rectClient.Width(), rectClient.Height());

	// 将位图选入内存 DC
	CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);

	// 用背景色填充内存 DC 的背景（避免残留图像）
	memDC.FillSolidRect(rectClient, pDoc->m_BackColor);

	// 调用原有的绘制函数，在内存 DC 上绘制
	Draw(&memDC);

	// 将内存 DC 的内容一次性复制到屏幕 DC
	pDC->BitBlt(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(),
		&memDC, 0, 0, SRCCOPY);

	// 绘制橡皮筋临时线（仅在绘图模式下且正在橡皮筋时）
	if (pDoc->DrawMod == LineMode && m_bRubberBanding)
	{
		// 创建几何虚线画笔（使用当前线条颜色，线宽与普通线相同）
		CPen rubberPen;
		LOGBRUSH lb = { BS_SOLID, pDoc->m_CurLineColor, 0 };
		DWORD dwStyle = PS_GEOMETRIC | PS_ENDCAP_ROUND | PS_JOIN_ROUND | PS_USERSTYLE;
		DWORD pattern[2] = { pDoc->m_LineWidth * 3, pDoc->m_LineWidth * 2 };
		rubberPen.CreatePen(dwStyle, pDoc->m_LineWidth, &lb, 2, pattern);

		CPen* pOldPenRubber = pDC->SelectObject(&rubberPen);

		// 将起点和终点从世界坐标转换为屏幕坐标（考虑偏移和缩放）
		SPoint startScreen = m_rubberStartWorld;
		SPoint endScreen = m_rubberEndWorld;
		SPoint::Getxy(startScreen, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);
		SPoint::Getxy(endScreen, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);

		// 绘制临时线段Y轴方向取反
		pDC->MoveTo(startScreen.m_x + pDoc->m_StateShow.m_dx,
			startScreen.m_y - pDoc->m_StateShow.m_dy);
		pDC->LineTo(endScreen.m_x + pDoc->m_StateShow.m_dx,
			endScreen.m_y - pDoc->m_StateShow.m_dy);

		pDC->SelectObject(pOldPenRubber);
	}

	// 高亮显示选中的线条（保留线型）
	if (pDoc->m_Data.GetSelectedCount() > 0) {
		// 创建几何画笔，使用文档当前的线型，颜色为当前高亮色，线宽加1
		CPen highlightPen;
		LOGBRUSH lb = { BS_SOLID, pDoc->m_CurLineColor, 0 };
		DWORD dwStyle = PS_GEOMETRIC | PS_ENDCAP_ROUND | PS_JOIN_ROUND;
		if (pDoc->m_LineStyle != 0) { // 非实线
			dwStyle |= PS_USERSTYLE;
			DWORD pattern[2];
			if (pDoc->m_LineStyle == 1) { // 虚线
				pattern[0] = pDoc->m_LineWidth * 3; // 划线长度
				pattern[1] = pDoc->m_LineWidth * 2; // 空白长度
			}
			else { // 点线
				pattern[0] = pDoc->m_LineWidth;     // 短划（点）
				pattern[1] = pDoc->m_LineWidth * 2; // 间隔
			}
			highlightPen.CreatePen(dwStyle, pDoc->m_LineWidth + 1, &lb, 2, pattern);
		}
		else { // 实线
			dwStyle |= PS_SOLID;
			highlightPen.CreatePen(dwStyle, pDoc->m_LineWidth + 1, &lb, 0, NULL);
		}

		CPen* pOldPen = pDC->SelectObject(&highlightPen);
		std::vector<int> selIndices = pDoc->m_Data.GetSelectedIndices();
		for (int idx : selIndices) {
			m_CLine* pLine = pDoc->m_Data.GetLine(idx);
			if (pLine)
				pLine->ShowLine(pDC, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);
		}
		pDC->SelectObject(pOldPen);
	}

	// 绘制角度辅助射线（单向，根据投影点方向决定正向或反向）
	if (m_bAngleSnapEnabled && m_bRubberBanding && m_bestAngleDirIndex != -1 && !m_bHighlightPoint)
	{
		CDrawCADDoc* pDoc = GetDocument();
		ASSERT_VALID(pDoc);
		if (!pDoc) return;

		double worldW = pDoc->m_w / pDoc->m_StateShow.m_r;
		double worldH = pDoc->m_h / pDoc->m_StateShow.m_r;
		double bigLen = 2.0 * sqrt(worldW * worldW + worldH * worldH); // 对角线两倍长

		const SPoint& start = m_rubberStartWorld;
		const SPoint& dir = s_angleDirs[m_bestAngleDirIndex];

		SPoint rayEnd;
		if (m_bestAngleDirPositive)
			rayEnd = SPoint(start.m_x + dir.m_x * bigLen, start.m_y + dir.m_y * bigLen, 0);
		else
			rayEnd = SPoint(start.m_x - dir.m_x * bigLen, start.m_y - dir.m_y * bigLen, 0);

		// 转换到屏幕坐标
		SPoint screenStart = start, screenEnd = rayEnd;
		SPoint::Getxy(screenStart, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);
		SPoint::Getxy(screenEnd, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);

		CPen guidePen;
		LOGBRUSH lb = { BS_SOLID, RGB(128,128,128), 0 };
		DWORD pattern[2] = { 4, 4 };
		guidePen.CreatePen(PS_GEOMETRIC | PS_USERSTYLE | PS_ENDCAP_ROUND, 1, &lb, 2, pattern);
		CPen* pOldPen = pDC->SelectObject(&guidePen);

		pDC->MoveTo((int)screenStart.m_x, (int)screenStart.m_y);
		pDC->LineTo((int)screenEnd.m_x, (int)screenEnd.m_y);

		pDC->SelectObject(pOldPen);
	}

	// 恢复内存 DC 的原始位图并清理资源
	memDC.SelectObject(pOldBitmap);

}


// CDrawCADView 打印


void CDrawCADView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CDrawCADView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CDrawCADView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CDrawCADView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CDrawCADView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	//OnContextMenu(this, point);
}

void CDrawCADView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CDrawCADView 诊断

#ifdef _DEBUG
void CDrawCADView::AssertValid() const
{
	CView::AssertValid();
}

void CDrawCADView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDrawCADDoc* CDrawCADView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDrawCADDoc)));
	return (CDrawCADDoc*)m_pDocument;
}
#endif //_DEBUG


// CDrawCADView 消息处理程序

void CDrawCADView::Draw(CDC* pDC)
{
	CDrawCADDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc); // 断言，确保Doc对象有效
	if (!pDoc) return;
	//创建画布
	CBrush* pOldBrush, BrushBk(pDoc->m_BackColor);		// 创建背景画刷
	pOldBrush = pDC->SelectObject(&BrushBk);			// 获取原画刷
	pDC->Rectangle(pDoc->m_Sorigin.m_x-1, pDoc->m_Sorigin.m_y-1, pDoc->m_w+2, pDoc->m_h+2);    // 绘制矩形 额外-1 +2使画布边缘更自然
	pDC->SelectObject(pOldBrush);						// 恢复原画刷

	// 绘制所有线条
	CPen* pOldPen;

	// 创建普通线条几何画笔
	CPen NewPen;
	LOGBRUSH lb = { BS_SOLID, pDoc->m_LineColor, 0 };
	DWORD dwStyle = PS_GEOMETRIC | PS_ENDCAP_ROUND | PS_JOIN_ROUND;
	if (pDoc->m_LineStyle != 0) // 虚线或点线
	{
		dwStyle |= PS_USERSTYLE;
		DWORD pattern[2];
		if (pDoc->m_LineStyle == 1) // 虚线
		{
			pattern[0] = pDoc->m_LineWidth * 3; // 划线长度
			pattern[1] = pDoc->m_LineWidth * 2; // 空白长度
		}
		else // 点线
		{
			pattern[0] = pDoc->m_LineWidth;     // 短划（点）
			pattern[1] = pDoc->m_LineWidth * 2; // 间隔
		}
		NewPen.CreatePen(dwStyle, pDoc->m_LineWidth, &lb, 2, pattern);
	}
	else
	{
		dwStyle |= PS_SOLID;
		NewPen.CreatePen(dwStyle, pDoc->m_LineWidth, &lb, 0, NULL);
	}
	pOldPen = pDC->SelectObject(&NewPen);
	pDoc->m_Data.ShowLine(pDC, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);

	// 绘制当前线条
	CPen NewPenCur;
	lb.lbColor = pDoc->m_CurLineColor; // 改用当前颜色
	dwStyle = PS_GEOMETRIC | PS_ENDCAP_ROUND | PS_JOIN_ROUND;
	if (pDoc->m_LineStyle != 0)
	{
		dwStyle |= PS_USERSTYLE;
		DWORD pattern[2];
		if (pDoc->m_LineStyle == 1)
		{
			pattern[0] = pDoc->m_LineWidth * 3;
			pattern[1] = pDoc->m_LineWidth * 2;
		}
		else
		{
			pattern[0] = pDoc->m_LineWidth;
			pattern[1] = pDoc->m_LineWidth * 2;
		}
		NewPenCur.CreatePen(dwStyle, pDoc->m_LineWidth, &lb, 2, pattern);
	}
	else
	{
		dwStyle |= PS_SOLID;
		NewPenCur.CreatePen(dwStyle, pDoc->m_LineWidth, &lb, 0, NULL);
	}
	pDC->SelectObject(&NewPenCur);
	pDoc->m_Data.ShowCurLine(pDC, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);


	if (pDoc->m_StateShow.m_bViewPoint) {													// 检查是否显示节点
		CPen NewPen(pDoc->m_LineStyle, 2, pDoc->m_LineColor);								// 创建点画笔
		pDC->SelectObject(&NewPen);															// 切换到点画笔
		pDoc->m_Data.ShowPoint(pDC, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);	// 绘制节点

		CPen NewPenCur(pDoc->m_LineStyle, 2, pDoc->m_CurLineColor);							// 创建当前节点画笔
		pDC->SelectObject(&NewPenCur);														// 切换到当前节点画笔
		CBrush Brush;
		Brush.CreateStockObject(NULL_BRUSH);												// 创建库存空画刷
		CBrush* pOldBrush;
		pOldBrush = pDC->SelectObject(&Brush);
		pDoc->m_Data.ShowCurPoint(pDC, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);// 绘制当前节点
	}

	// 绘制高亮吸附节点（仅在非拖拽模式下）
	if (m_bHighlightPoint && m_dragMode == None)
	{
		m_CLine* pLine = pDoc->m_Data.GetLine(m_highlightLineIdx);
		if (pLine && m_highlightPointIdx >= 0 && m_highlightPointIdx < pLine->GetNum())
		{
			SPoint worldPt = pLine->GetPoint(m_highlightPointIdx);
			SPoint base = worldPt;
			SPoint::Getxy(base, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);
			double screenX = base.m_x + pDoc->m_StateShow.m_dx;
			double screenY = base.m_y - pDoc->m_StateShow.m_dy;

			CPen highlightPen(PS_SOLID, 2, pDoc->m_CurLineColor);
			CPen* pOldPen = pDC->SelectObject(&highlightPen);
			CBrush* pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);

			int d = 12;  // 高亮矩形半宽，比普通节点（通常 d=10）稍大
			pDC->Rectangle(
				static_cast<int>(screenX - d),
				static_cast<int>(screenY - d),
				static_cast<int>(screenX + d),
				static_cast<int>(screenY + d));

			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBrush);
		}
	}

	pDC->SelectObject(pOldPen);							// 恢复原画笔

	if (pDoc)
	{
		int Num = pDoc->m_Data.GetLineSum();
		double r = pDoc->m_StateShow.m_r;
		double dx = pDoc->m_StateShow.m_dx;
		double dy = pDoc->m_StateShow.m_dy;
		CString str = pDoc->GetPathName();
		if (str.IsEmpty())
			str = _T("未保存");  // 或者 _T("无文件")
		// 格式化字符串
		CString strCoord;
		strCoord.Format(_T("图形总数: %d ; 缩放倍数: %.2f; 横向偏移: %.2f ; 纵向偏移: %.2f ; "), Num, r, dx, dy);
		strCoord += _T("文件路径: ") + str;

		// 获取主框架并更新状态栏
		CMainFrame* pFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
		if (pFrame)
		{
			pFrame->SetStatusBarPaneText(ID_STATUSBAR_PANE1, strCoord);
		}
	}
}

void CDrawCADView::MoveLine(double x, double y)
{
	CDrawCADDoc* pDoc = GetDocument();
	pDoc->m_StateShow.m_dx += x;
	pDoc->m_StateShow.m_dy -= y; //默认正值为向左上
}

void CDrawCADView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	ClearHighlight();                    // 点击时清除高亮
	point = SnapToPoint(point);          // 吸附处理

	CDrawCADDoc* pDoc = GetDocument();
	if (point.x < pDoc->m_Sorigin.m_x || point.x >= pDoc->m_w || point.y < pDoc->m_Sorigin.m_y || point.y >= pDoc->m_h)
		return;

	// 调整点击坐标，考虑显示偏移
	point.x -= pDoc->m_StateShow.m_dx;
	point.y += pDoc->m_StateShow.m_dy;

	// 创建节点对象并进行坐标转换
	SPoint tPoint = { (double)point.x, (double)point.y ,0 };
	SPoint::GetXY(tPoint, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);

	if (pDoc->DrawMod == LineMode) {
		m_CData::ClearClipboard();	// 清空内部剪贴板

		// 判断是否已有当前线条（即不是第一次添加点）
		bool bHasCurLine = (pDoc->m_Data.GetCurLine() != nullptr);

		SPoint pointToAdd;
		if (bHasCurLine && m_bAngleSnapEnabled && !m_bHighlightPoint) {
			// 已有当前线条，且角度约束生效且无高亮节点：使用橡皮筋终点（约束后的点）
			pointToAdd = m_rubberEndWorld;
		}
		else {
			// 否则：使用原始鼠标点（第一次添加点，或角度约束未生效/有高亮节点）
			pointToAdd = tPoint;
		}

		// 保存撤销状态并添加节点
		pDoc->SaveStateForUndo();
		pDoc->m_Data.AddPoint(pointToAdd);

		// 更新橡皮筋状态：以刚添加的点作为下一段的起点
		m_bRubberBanding = true;
		m_rubberStartWorld = pointToAdd;
		m_rubberEndWorld = pointToAdd;          // 暂设为起点，避免立即画线

		// 根据当前鼠标位置重新计算橡皮筋终点（应用角度约束或吸附）
		UpdateRubberBandEnd(point);

		// 清除高亮并刷新视图
		ClearHighlight();
		Invalidate();
	}
	else if(pDoc->DrawMod == SelectMode){
		// 选择模式：设置当前节点/曲线

		 // 判断 Ctrl 键是否按下
		bool bCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;

		//double threshold = 10.0 / pDoc->m_StateShow.m_r; // 世界坐标阈值

		double threshold = (double)SNAP_TOLERANCE / pDoc->m_StateShow.m_r;
		// 1. 首先尝试查找最近的节点
		int lineIdx, pointIdx;
		if (pDoc->m_Data.FindNearestPoint(tPoint, threshold, lineIdx, pointIdx))
		{
			pDoc->m_Data.SetCurLine(tPoint, 5);
			// 进入节点拖拽模式
			m_dragMode = DragPoint;
			m_dragLineIdx = lineIdx;
			m_dragPointIdx = pointIdx;
			m_CLine* pLine = pDoc->m_Data.GetLine(lineIdx);
			m_dragStartWorld = pLine->GetPoint(pointIdx);      // 节点原始坐标
			m_dragStartMouseWorld = tPoint;                     // 鼠标按下时的世界坐标
			pDoc->SaveStateForUndo();                            // 保存撤销点
			SetCapture();
			Invalidate();
			return; // 直接返回，不再执行后续线条查找
		}

		// 2. 未找到节点，再查找线条
		int lineIdx2, segIdx;
		SPoint foot;

		// 尝试查找最近的线条
		if (pDoc->m_Data.FindNearestLine(tPoint, threshold, lineIdx, segIdx, foot))
		{
			// 根据 Ctrl 键更新选择集
			if (bCtrl) {
				// Ctrl+点击：切换选中状态
				if (pDoc->m_Data.IsSelected(lineIdx))
					pDoc->m_Data.RemoveSelected(lineIdx);
				else
					pDoc->m_Data.AddSelected(lineIdx);
			}
			else {
				// 非Ctrl：清除其他，只选中当前线（如果它尚未选中则选中，否则保持原样）
				if (!pDoc->m_Data.IsSelected(lineIdx)) {
					pDoc->m_Data.ClearSelected();
					pDoc->m_Data.AddSelected(lineIdx);
				}
				// 如果已经选中，选择集不变（符合“点击已选中对象开始拖拽”的预期）
			}
			// 判断是否应进入拖拽模式：当前线条最终是选中的
			if (pDoc->m_Data.IsSelected(lineIdx)) {
				// 进入多选拖拽模式
				m_dragMode = DragMultiLine;
				// 保存所有选中线条的原始点
				m_selectedOrigPoints.clear();
				std::vector<int> selIndices = pDoc->m_Data.GetSelectedIndices();
				for (int idx : selIndices) {
					m_CLine* pLine = pDoc->m_Data.GetLine(idx);
					int n = pLine->GetNum();
					std::vector<SPoint> pts;
					for (int i = 0; i < n; ++i)
						pts.push_back(pLine->GetPoint(i));
					m_selectedOrigPoints[idx] = pts;
				}
				m_dragStartMouseWorld = tPoint;   // 记录鼠标按下时的世界坐标
				pDoc->SaveStateForUndo();          // 保存撤销点
				SetCapture();
				Invalidate();
				return;
			}
			else {
				// 未进入拖拽（例如Ctrl+点击取消选中），仅刷新视图
				Invalidate();
			}
		}
		else
		{
			// 点击空白处，清除所有选中
			pDoc->m_Data.ClearSelected();
			Invalidate();
		}
	}

	// 刷新显示
	CClientDC dc(this);
	Draw(&dc);
	pDoc->SetModifiedFlag(TRUE);  // 标记文档已修改

	CView::OnLButtonDown(nFlags, point);
}

void CDrawCADView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDrawCADDoc* pDoc = GetDocument();
	pDoc->m_Data.EndLine();					// 结束当前线条
	ClearRubberBand();						// 同时清除 m_bAngleSnapEnabled 和 m_bestAngleDirIndex
	ClearHighlight();						// 结束线条时清除高亮
	Invalidate();							// 刷新，擦除临时线
	CView::OnRButtonDown(nFlags, point);
}

void CDrawCADView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	CDrawCADDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	GetClientRect(pDoc->m_ViewRect);
	pDoc->m_w = pDoc->m_ViewRect.Width();
	pDoc->m_h = pDoc->m_ViewRect.Height();

	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame != nullptr) // 必须检查指针有效性！
	{
		pMainFrame->GetFrameSize();
		pDoc->m_x2 = pDoc->m_FrameRect.Width();
		pDoc->m_y2 = pDoc->m_FrameRect.Height();
	}
}

void CDrawCADView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类

	// 获取文档指针
	CDrawCADDoc* pDoc = GetDocument();
	if (pDoc == nullptr) return;		// 文档指针无效则退出

	// 获取当前视图的客户区尺寸
	GetClientRect(pDoc->m_ViewRect);			// 获取视图客户区坐标
	pDoc->m_w = pDoc->m_ViewRect.Width();
	pDoc->m_h = pDoc->m_ViewRect.Height();

	// 获取主框架指针
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame != nullptr)			// 检查指针有效性
	{
		pMainFrame->GetFrameSize();
		pDoc->m_x2 = pDoc->m_FrameRect.Width();
		pDoc->m_y2 = pDoc->m_FrameRect.Height();
	}
	else
	{									// 框架指针无效时赋默认值，避免崩溃								
		pDoc->m_FrameRect.SetRectEmpty();
		pDoc->m_x2 = pDoc->m_y2 = 0;
	}
	CMainFrame* pFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pFrame)
		pFrame->UpdateScaleCombo();
}


void CDrawCADView::OnLinecolor()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADDoc* pDoc = GetDocument();
	CColorDialog colorDlg(pDoc->m_LineColor, CC_FULLOPEN | CC_ANYCOLOR);           // 构造颜色对话框，传入初始颜色值

	if (IDOK == colorDlg.DoModal())         // 显示颜色对话框，并判断是否点击了“确定”
	{
		pDoc->m_LineColor = colorDlg.GetColor();        // 获取颜色对话框中选择的颜色值
	}
	Invalidate();
}

void CDrawCADView::OnCurlinecolor()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADDoc* pDoc = GetDocument();
	CColorDialog colorDlg(pDoc->m_CurLineColor, CC_FULLOPEN | CC_ANYCOLOR);           // 构造颜色对话框，传入初始颜色值

	if (IDOK == colorDlg.DoModal())         // 显示颜色对话框，并判断是否点击了“确定”
	{
		pDoc->m_CurLineColor = colorDlg.GetColor();        // 获取颜色对话框中选择的颜色值
	}
	Invalidate();
}

void CDrawCADView::OnBackcolor()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADDoc* pDoc = GetDocument();
	CColorDialog colorDlg(pDoc->m_BackColor, CC_FULLOPEN | CC_ANYCOLOR);           // 构造颜色对话框，传入初始颜色值

	if (IDOK == colorDlg.DoModal())         // 显示颜色对话框，并判断是否点击了“确定”
	{
		pDoc->m_BackColor = colorDlg.GetColor();        // 获取颜色对话框中选择的颜色值
	}
	Invalidate();
}

void CDrawCADView::OnDelpoint()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADDoc* pDoc = GetDocument();
	pDoc->SaveStateForUndo();
	pDoc->m_Data.DelPoint();  // 删除节点
	Invalidate();  // 刷新
}

void CDrawCADView::OnUpdateDelpoint(CCmdUI* pCmdUI)
{
	CDrawCADDoc* pDoc = GetDocument();
	BOOL bEnable = FALSE;
	if (pDoc)
	{
		m_CLine* pCurLine = pDoc->m_Data.GetCurLine();
		// 当前线条存在且当前节点索引有效（即有选中节点）
		bEnable = (pCurLine != nullptr && pCurLine->GetCurPointIndex() != -1);
	}
	pCmdUI->Enable(bEnable);
}

void CDrawCADView::OnDelline()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc) return;

	// 无任何可删除的线条时直接返回
	if (pDoc->m_Data.GetSelectedCount() == 0 && pDoc->m_Data.GetCurLine() == nullptr)
		return;

	pDoc->SaveStateForUndo();

	int deletedCount = 0;
	if (pDoc->m_Data.GetSelectedCount() > 0)
	{
		deletedCount = pDoc->m_Data.DeleteSelectedLines();
	}
	else
	{
		if (pDoc->m_Data.DelLine())
			deletedCount = 1;
	}

	if (deletedCount > 0)
	{
		pDoc->SetModifiedFlag(TRUE);
		Invalidate();
	}
}

void CDrawCADView::OnUpdateDelline(CCmdUI* pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	CDrawCADDoc* pDoc = GetDocument();
	BOOL bEnable = (pDoc && (pDoc->m_Data.GetSelectedCount() > 0 || pDoc->m_Data.GetCurLine() != nullptr));
	pCmdUI->Enable(bEnable);
}


void CDrawCADView::OnAdapt()
{
	// TODO: 在此添加命令处理程序代码
	CDrawCADDoc* pDoc = GetDocument();
    if (!pDoc) return;					// 文档指针无效则退出
    CRect rect = pDoc->m_Data.GetmRect();
	rect.NormalizeRect();				//使矩形符合规范
	if (rect.IsRectEmpty()) return;		// 无图形时不做任何事

    // 图形包围盒尺寸
    double width = rect.Width();       // >0
    double height = rect.Height();     // >0

    // 图形中心点世界坐标
	double centerX = rect.CenterPoint().x;
	double centerY = rect.CenterPoint().y;

    // 窗口中心（视图客户区中心）
    double viewCx = pDoc->m_w / 2.0;
    double viewCy = pDoc->m_h / 2.0;

    // 计算缩放比例，使图形完全适应窗口（保持纵横比）
    double scale;
	if (width / height > pDoc->m_w / pDoc->m_h)
		scale = pDoc->m_w / width;   // 屏幕宽度 / 世界宽度
	else
		scale = pDoc->m_h / height;

    // 设置缩放
    pDoc->m_StateShow.m_r = scale;

    // 计算平移量，使图形中心对准窗口中心
	pDoc->m_StateShow.m_dx = viewCx - centerX * scale;   
	pDoc->m_StateShow.m_dy = -viewCy - centerY * scale;

	CMainFrame* pFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());	//更新显示倍率
	if (pFrame)
		pFrame->UpdateScaleCombo();

	Invalidate();
}

void CDrawCADView::CenterView()				//居中显示
{
	// TODO: 在此处添加实现代码.
	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc) return;					// 文档指针无效则退出
	CRect rect = pDoc->m_Data.GetmRect();
	rect.NormalizeRect();				//使矩形符合规范
	if (rect.IsRectEmpty())
	{
		Invalidate();
		return;
	}
	// 图形包围盒尺寸
	double width = rect.Width();       // >0
	double height = rect.Height();     // >0
	// 图形中心点世界坐标
	double centerX = rect.CenterPoint().x;
	double centerY = rect.CenterPoint().y;
	// 窗口中心（视图客户区中心）
	double viewCx = pDoc->m_w / 2.0;
	double viewCy = pDoc->m_h / 2.0;
	// 计算平移量，使图形中心对准窗口中心
	pDoc->m_StateShow.m_dx = viewCx - centerX * pDoc->m_StateShow.m_r;
	pDoc->m_StateShow.m_dy = -viewCy - centerY * pDoc->m_StateShow.m_r;
	Invalidate();
}

void CDrawCADView::ZoomAtPoint(CPoint point, double factor)// 在特定点放大
{
	// TODO: 在此处添加实现代码.
	CDrawCADDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	// 获取当前缩放状态
	double r = pDoc->m_StateShow.m_r;
	double dx = pDoc->m_StateShow.m_dx;
	double dy = pDoc->m_StateShow.m_dy;

	// 计算鼠标点对应的世界坐标 (wx, wy)
	double wx = (point.x - dx) / r;
	double wy = -(point.y + dy) / r;   // 注意负号

	// 计算新缩放倍率，并限制范围
	double new_r = r * factor;
	// 可加入最大/最小限制（如 MainFrm.cpp 中的 MAX_SCALE / MIN_SCALE）
	if (new_r > MAX_SCALE) new_r = MAX_SCALE;
	if (new_r < MIN_SCALE) new_r = MIN_SCALE;

	// 计算新平移量，使 (wx, wy) 仍位于 (point.x, point.y)
	double new_dx = point.x - wx * new_r;
	double new_dy = -point.y - wy * new_r;

	// 更新文档状态
	pDoc->m_StateShow.m_r = new_r;
	pDoc->m_StateShow.m_dx = new_dx;
	pDoc->m_StateShow.m_dy = new_dy;

	CMainFrame* pFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());	//更新显示倍率
	if (pFrame)
		pFrame->UpdateScaleCombo();

	// 刷新视图
	Invalidate();
}

BOOL CDrawCADView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	//if (nFlags & MK_CONTROL)   // 按住 Ctrl 键时缩放
	//{
		CPoint clientPt = pt;
		ScreenToClient(&clientPt);

		CRect rect;
		GetClientRect(&rect);
		if (rect.PtInRect(clientPt))
		{
			m_lastZoomPoint = clientPt;	// 记录最后一次缩放位置
			m_bHasZoomPoint = true;

			double factor = (zDelta > 0) ? GetDocument()->m_nZoom : 1.0 / GetDocument()->m_nZoom;
			ZoomAtPoint(clientPt, factor);

			return TRUE;
		}
	//}

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}



void CDrawCADView::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	m_bDragging = true;	// 记录拖拽起始点和当前偏移量
	m_dragStart = point;
	CDrawCADDoc* pDoc = GetDocument();
	if (pDoc)
	{
		m_dragStartDx = pDoc->m_StateShow.m_dx;
		m_dragStartDy = pDoc->m_StateShow.m_dy;
	}
	SetCapture();		// 捕获鼠标，保证移出窗口后仍能接收移动消息

	CView::OnMButtonDown(nFlags, point);
}

void CDrawCADView::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
		
	if (m_bDragging)
	{
		m_bDragging = false;
		ReleaseCapture();   // 释放鼠标捕获
	}

	CView::OnMButtonUp(nFlags, point);
}

void CDrawCADView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	point = SnapToPoint(point);          // 吸附处理
	Invalidate();                        // 确保高亮能立即显示

	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc) return;

	// 拖拽窗口
	if (m_bDragging && (nFlags & MK_MBUTTON)) // 确保中键仍按下
	{
		// 计算鼠标位移量
		int deltaX = point.x - m_dragStart.x;
		int deltaY = point.y - m_dragStart.y;

		// 更新偏移：dx 增加 deltaX，dy 减少 deltaY（根据坐标转换公式推导）
		pDoc->m_StateShow.m_dx = m_dragStartDx + deltaX;
		pDoc->m_StateShow.m_dy = m_dragStartDy - deltaY;

		// 刷新视图
		Invalidate();
	}

	// 拖拽节点
	if (m_dragMode == DragPoint && (nFlags & MK_LBUTTON))
	{
		// 计算当前鼠标的世界坐标
		CPoint adjPoint = point;
		adjPoint.x -= pDoc->m_StateShow.m_dx;
		adjPoint.y += pDoc->m_StateShow.m_dy;
		SPoint currentMouseWorld(adjPoint.x, adjPoint.y, 0);
		SPoint::GetXY(currentMouseWorld, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);

		// 计算世界位移
		double worldDeltaX = currentMouseWorld.m_x - m_dragStartMouseWorld.m_x;
		double worldDeltaY = currentMouseWorld.m_y - m_dragStartMouseWorld.m_y;

		// 更新节点坐标
		m_CLine* pLine = pDoc->m_Data.GetLine(m_dragLineIdx);
		SPoint newPt = m_dragStartWorld;
		newPt.m_x += worldDeltaX;
		newPt.m_y += worldDeltaY;
		pLine->SetPoint(m_dragPointIdx, newPt);

		pDoc->SetModifiedFlag(TRUE);
		Invalidate();
		return;
	}

	// 拖拽多选线条
	if (m_dragMode == DragMultiLine && (nFlags & MK_LBUTTON)) {
		// 计算当前鼠标的世界坐标
		CPoint adjPoint = point;
		adjPoint.x -= pDoc->m_StateShow.m_dx;
		adjPoint.y += pDoc->m_StateShow.m_dy;
		SPoint currentMouseWorld(adjPoint.x, adjPoint.y, 0);
		SPoint::GetXY(currentMouseWorld, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);

		double worldDeltaX = currentMouseWorld.m_x - m_dragStartMouseWorld.m_x;
		double worldDeltaY = currentMouseWorld.m_y - m_dragStartMouseWorld.m_y;

		// 更新所有选中的线条
		for (auto& pair : m_selectedOrigPoints) {
			int lineIdx = pair.first;
			const std::vector<SPoint>& origPts = pair.second;
			m_CLine* pLine = pDoc->m_Data.GetLine(lineIdx);
			int n = (int)origPts.size();
			for (int i = 0; i < n; ++i) {
				SPoint newPt(origPts[i].m_x + worldDeltaX,
					origPts[i].m_y + worldDeltaY,
					i);   // 保留序号
				pLine->SetPoint(i, newPt);
			}
		}
		pDoc->SetModifiedFlag(TRUE);
		Invalidate();
		return;
	}

	// 检测 Shift 键状态，实时更新角度限制标志
	bool bShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	if (bShift != m_bAngleSnapEnabled)
	{
		m_bAngleSnapEnabled = bShift;
		RefreshRubberBand();   // 立即用当前鼠标位置刷新
	}

	// 执行吸附（会更新 m_bHighlightPoint 等）
	point = SnapToPoint(point);

	// 更新橡皮筋终点（内部会处理角度约束）
	if (pDoc->DrawMod == LineMode && m_bRubberBanding)
	{
		UpdateRubberBandEnd(point);
	}

	// 将屏幕坐标转换为世界坐标
	CPoint adjPoint = point;
	adjPoint.x -= pDoc->m_StateShow.m_dx;
	adjPoint.y += pDoc->m_StateShow.m_dy;
	SPoint worldPt(adjPoint.x, adjPoint.y, 0);
	SPoint::GetXY(worldPt, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);

	// 格式化字符串
	CString strCoord;
	strCoord.Format(_T("鼠标世界坐标: (%.2f, %.2f)"), worldPt.m_x, worldPt.m_y);

	// 获取主框架并更新状态栏（例如使用第一个窗格）
	CMainFrame* pFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pFrame)
	{
		pFrame->SetStatusBarPaneText(ID_STATUSBAR_PANE2, strCoord);
	}

	//// 将屏幕坐标转换为世界坐标（已在函数前半部分计算 worldPt）	橡皮筋
	//if (pDoc->DrawMod == LineMode && m_bRubberBanding)
	//{
	//	// 更新橡皮筋终点（世界坐标）
	//	m_rubberEndWorld = worldPt;
	//	Invalidate();   // 触发重绘，显示临时线
	//}

	CView::OnMouseMove(nFlags, point);
}

BOOL CDrawCADView::OnEraseBkgnd(CDC* pDC)	//配合双缓冲 减少闪烁
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	//return CView::OnEraseBkgnd(pDC);
	return TRUE;
}

void CDrawCADView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_dragMode == DragPoint || m_dragMode == DragLine || m_dragMode == DragMultiLine) {
		ReleaseCapture();
		m_dragMode = None;
		//m_lineOrigPoints.clear();      // 如果仍使用单线拖拽，可保留
		m_selectedOrigPoints.clear();  // 清除多选拖拽临时数据
	}

	CView::OnLButtonUp(nFlags, point);
}

void CDrawCADView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	point = SnapToPoint(point);          // 吸附处理

	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc || pDoc->DrawMod != SelectMode)
	{
		CView::OnLButtonDblClk(nFlags, point);
		return;
	}

	// 转换世界坐标
	SPoint worldPt;
	worldPt.m_x = (point.x - pDoc->m_StateShow.m_dx) / pDoc->m_StateShow.m_r;
	worldPt.m_y = -(point.y + pDoc->m_StateShow.m_dy) / pDoc->m_StateShow.m_r;

	double threshold = 5.0 / pDoc->m_StateShow.m_r;		//设置检查阈值
	int lineIdx, segIdx;
	SPoint foot;

	if (pDoc->m_Data.FindNearestLine(worldPt, threshold, lineIdx, segIdx, foot))
	{
		m_CData::ClearClipboard(); //清空内部剪贴板
		pDoc->SaveStateForUndo();

		m_CLine* pLine = pDoc->m_Data.GetLine(lineIdx);
		// 在 segIdx 和 segIdx+1 之间插入 foot
		pLine->InsertPoint(segIdx, foot); 

		pDoc->SetModifiedFlag(TRUE);
		Invalidate();
	}
}

void CDrawCADView::ClearRubberBand()
{
	m_bRubberBanding = false;
	m_bAngleSnapEnabled = false;
	m_bestAngleDirIndex = -1;
	m_bestAngleDirPositive = true; // 重置为默认
}

CPoint CDrawCADView::SnapToPoint(CPoint point)
{
	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc || m_dragMode != None)
	{
		ClearHighlight();
		return point;
	}

	// 如果吸附被禁用，直接返回原始点，并清除高亮
	if (!pDoc->m_bSnapEnabled)
	{
		ClearHighlight();
		return point;
	}

	double minDist = SNAP_TOLERANCE;
	CPoint snappedPoint = point;
	int foundLine = -1, foundPoint = -1;

	int lineCount = pDoc->m_Data.GetLineSum();
	for (int i = 0; i < lineCount; i++)
	{
		m_CLine* pLine = pDoc->m_Data.GetLine(i);
		if (!pLine) continue;
		int pointCount = pLine->GetNum();
		for (int j = 0; j < pointCount; j++)
		{
			SPoint worldPt = pLine->GetPoint(j);
			SPoint base = worldPt;
			SPoint::Getxy(base, pDoc->m_Worigin, pDoc->m_Sorigin, pDoc->m_StateShow);
			double screenX = base.m_x + pDoc->m_StateShow.m_dx;
			double screenY = base.m_y - pDoc->m_StateShow.m_dy;

			double dx = point.x - screenX;
			double dy = point.y - screenY;
			double dist = sqrt(dx * dx + dy * dy);
			if (dist < minDist)
			{
				minDist = dist;
				snappedPoint.x = static_cast<int>(screenX + 0.5);
				snappedPoint.y = static_cast<int>(screenY + 0.5);
				foundLine = i;
				foundPoint = j;
			}
		}
	}

	if (foundLine != -1)
	{
		// 检查是否是绘画模式下当前线条的最后一个点
		bool isLastPointOfCurrentLine = false;
		//if (pDoc->DrawMod == LineMode && pDoc->m_Data.GetCurLine() != nullptr)
		//{
		//	int lastIdx = pDoc->m_Data.GetCurLine()->GetNum() - 1;
		//	if (pDoc->m_Data.GetCurLine()->GetLineNum() == foundLine && foundPoint == lastIdx)
		//	{
		//		isLastPointOfCurrentLine = true;
		//	}
		//}

		//if (isLastPointOfCurrentLine)
		//{
		//	ClearHighlight();   // 最后一个点不高亮
		//}
		bool isCurrentPoint = false;
		if (pDoc->m_Data.GetCurLine() != nullptr && pDoc->m_Data.GetCurLine()->GetLineNum() == foundLine)
		{
			int curIdx = pDoc->m_Data.GetCurLine()->GetCurPointIndex();
			if (curIdx == foundPoint)
				isCurrentPoint = true;
		}

		if (isCurrentPoint)
		{
			ClearHighlight();   // 当前节点不高亮
		}
		else
		{
			m_bHighlightPoint = true;
			m_highlightLineIdx = foundLine;
			m_highlightPointIdx = foundPoint;
		}
	}
	else
	{
		ClearHighlight();
	}

	return snappedPoint;
}

bool CDrawCADView::GetMouseWorldPoint(CPoint& screenPt, SPoint& worldPt) const
{
	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc) return false;

	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	CRect clientRect;
	GetClientRect(clientRect);
	if (!clientRect.PtInRect(pt))
		return false;

	screenPt = pt;
	// 转换为世界坐标
	double wx = (pt.x - pDoc->m_StateShow.m_dx) / pDoc->m_StateShow.m_r;
	double wy = -(pt.y + pDoc->m_StateShow.m_dy) / pDoc->m_StateShow.m_r;
	worldPt = SPoint(wx, wy, 0);
	return true;
}

void CDrawCADView::ClearHighlight()
{
	m_bHighlightPoint = false;
	m_highlightLineIdx = -1;
	m_highlightPointIdx = -1;
}

void CDrawCADView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc) return;

	// 根据文档状态更新橡皮筋
	if (pDoc->DrawMod == LineMode && pDoc->m_Data.GetCurLine() != nullptr && pDoc->m_Data.GetCurLine()->GetNum() > 0)
	{
		// 有当前线条且至少有一个点，激活橡皮筋
		m_bRubberBanding = true;
		// 起点为当前线条最后一个点
		int lastIdx = pDoc->m_Data.GetCurLine()->GetNum() - 1;
		m_rubberStartWorld = pDoc->m_Data.GetCurLine()->GetPoint(lastIdx);

		// 尝试获取当前鼠标位置作为终点
		CPoint screenPt;
		SPoint worldPt;
		if (GetMouseWorldPoint(screenPt, worldPt))
		{
			m_rubberEndWorld = worldPt;
		}
		else
		{
			// 鼠标不在客户区内，终点设为起点（这样不会画线）
			m_rubberEndWorld = m_rubberStartWorld;
		}
	}
	else
	{
		// 否则关闭橡皮筋
		m_bRubberBanding = false;
	}

	// 清除高亮
	ClearHighlight();

	// 如果正在拖拽，取消拖拽（文档数据已变）
	if (m_dragMode != None)
	{
		ReleaseCapture();
		m_dragMode = None;
		m_lineOrigPoints.clear();
		m_selectedOrigPoints.clear();   // 清除多选拖拽的临时数据
	}

	Invalidate();  // 强制重绘
}

//粘贴
void CDrawCADView::OnEditPaste()
{
	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc) return;

	// 获取鼠标位置的世界坐标
	SPoint targetWorld;
	CPoint screenPt;
	bool bHasMouse = GetMouseWorldPoint(screenPt, targetWorld);

	// 调用文档的粘贴函数，传递鼠标世界坐标（如果有效）
	pDoc->PasteFromClipboard(bHasMouse ? &targetWorld : nullptr);
}

void CDrawCADView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_CData::HasClipboardData());
}


void CDrawCADView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)	//esc取消选择
{
	//Delete
	if (nChar == VK_DELETE)
	{
		CDrawCADDoc* pDoc = GetDocument();
		if (!pDoc) return;

		// 仅在选择模式下允许删除
		if (pDoc->DrawMod != SelectMode)
		{
			CView::OnKeyDown(nChar, nRepCnt, nFlags);
			return;
		}

		// 优先删除当前节点（如果有选中节点）
		if (pDoc->m_Data.GetCurLine() != nullptr && pDoc->m_Data.GetCurLine()->GetCurPointIndex() != -1)
		{
			pDoc->SaveStateForUndo();

			int lineIdx = pDoc->m_Data.GetCurLine()->GetLineNum();
			// 删除节点
			pDoc->m_Data.DelPoint();

			// 检查该线条是否变为空，若是则删除整条线
			m_CLine* pLine = pDoc->m_Data.GetLine(lineIdx);
			if (pLine && pLine->GetNum() == 0)
			{
				// 设置当前线条为要删除的线（DelLine 会删除当前线条）
				pDoc->m_Data.SetCurLineByIndex(lineIdx);
				pDoc->m_Data.DelLine();
			}
			else if (pLine)
			{
				// 清除当前节点标记（节点已被删除）
				pLine->ClearCurPoint();
			}

			pDoc->SetModifiedFlag(TRUE);
			Invalidate();
		}
		// 否则删除所有选中的线条
		else if (pDoc->m_Data.GetSelectedCount() > 0)
		{
			pDoc->SaveStateForUndo();
			pDoc->m_Data.DeleteSelectedLines();
			pDoc->SetModifiedFlag(TRUE);
			Invalidate();
		}
		// 若无任何选中，不执行操作
		return; // 已处理 Delete 键，不再传递
	}
	//shift
	if (nChar == VK_SHIFT)
	{
		if (!m_bAngleSnapEnabled)
		{
			m_bAngleSnapEnabled = true;
			RefreshRubberBand();
		}
	}
	//esc
	if (nChar == VK_ESCAPE)
	{
		CDrawCADDoc* pDoc = GetDocument();
		if (!pDoc) return;

		if (pDoc->DrawMod == LineMode)
		{
			pDoc->m_Data.EndLine();
			m_bRubberBanding = false;
			ClearHighlight();
			Invalidate();
		}
		else if (pDoc->DrawMod == SelectMode)
		{
			pDoc->DeselectAll();
			ClearHighlight();
			Invalidate();
		}
		return; // 已处理 ESC，不再传递
	}

	//tab切换模式
	if (nChar == VK_TAB)
	{
		CDrawCADDoc* pDoc = GetDocument();
		if (!pDoc) return;

		// 切换模式（绘图 ? 选择）
		pDoc->DrawMod = (pDoc->DrawMod == LineMode) ? SelectMode : LineMode;

		// 清除橡皮筋和高亮状态
		ClearRubberBand();
		ClearHighlight();

		// 更新 Ribbon 上的组合框
		CMainFrame* pFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
		if (pFrame)
		{
			int nIndex = (pDoc->DrawMod == LineMode) ? 0 : 1;
			pFrame->SetModeComboSelection(nIndex);
		}

		// 刷新视图
		Invalidate();

		return;  // Tab 已处理，不再传递
	}

	//移动
	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc) return;

	double dx = 0, dy = 0;
	double step = pDoc->m_MoveStep; // 世界毫米步长

	switch (nChar)
	{
	case VK_LEFT:  dx = -step; break;
	case VK_RIGHT: dx = step; break;
	case VK_UP:    dy = step; break;
	case VK_DOWN:  dy = -step; break;
	default:
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	// 选择模式且有选中对象时移动对象，否则移动视图
	if (pDoc->DrawMod == SelectMode && pDoc->m_Data.GetSelectedCount() > 0)
	{
		// 优先移动所有选中的线条（无论是否有节点选中）
		pDoc->SaveStateForUndo();

		std::vector<int> selIndices = pDoc->m_Data.GetSelectedIndices();
		for (int idx : selIndices)
		{
			m_CLine* pLine = pDoc->m_Data.GetLine(idx);
			int n = pLine->GetNum();
			for (int i = 0; i < n; i++)
			{
				SPoint pt = pLine->GetPoint(i);
				pt.m_x += dx;
				pt.m_y += dy;
				pLine->SetPoint(i, pt);
			}
		}
		pDoc->SetModifiedFlag(TRUE);
		Invalidate();
	}
	else if (pDoc->DrawMod == SelectMode &&
		pDoc->m_Data.GetCurLine() &&
		pDoc->m_Data.GetCurLine()->GetCurPointIndex() != -1)
	{
		// 无选中线条但有选中节点，移动该节点
		pDoc->SaveStateForUndo();

		int lineIdx = pDoc->m_Data.GetCurLine()->GetLineNum();
		int pointIdx = pDoc->m_Data.GetCurLine()->GetCurPointIndex();
		SPoint pt = pDoc->m_Data.GetLine(lineIdx)->GetPoint(pointIdx);
		pt.m_x += dx;
		pt.m_y += dy;
		pDoc->m_Data.GetLine(lineIdx)->SetPoint(pointIdx, pt);

		pDoc->SetModifiedFlag(TRUE);
		Invalidate();
	}
	else
	{
		// 移动视图（保留原有功能）
		pDoc->m_StateShow.m_dx += dx * pDoc->m_StateShow.m_r;
		pDoc->m_StateShow.m_dy += dy * pDoc->m_StateShow.m_r; // 原代码符号保持不变
		Invalidate();
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDrawCADView::MoveByStep(double dx, double dy)
{
	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc) return;

	// 选择模式且有选中线条 -> 移动所有选中线条
	if (pDoc->DrawMod == SelectMode && pDoc->m_Data.GetSelectedCount() > 0)
	{
		pDoc->SaveStateForUndo();

		std::vector<int> selIndices = pDoc->m_Data.GetSelectedIndices();
		for (int idx : selIndices)
		{
			m_CLine* pLine = pDoc->m_Data.GetLine(idx);
			int n = pLine->GetNum();
			for (int i = 0; i < n; i++)
			{
				SPoint pt = pLine->GetPoint(i);
				pt.m_x += dx;
				pt.m_y += dy;
				pLine->SetPoint(i, pt);
			}
		}
		pDoc->SetModifiedFlag(TRUE);
		Invalidate();
	}
	// 选择模式且仅选中一个节点 -> 移动该节点
	else if (pDoc->DrawMod == SelectMode &&
		pDoc->m_Data.GetCurLine() &&
		pDoc->m_Data.GetCurLine()->GetCurPointIndex() != -1)
	{
		pDoc->SaveStateForUndo();

		int lineIdx = pDoc->m_Data.GetCurLine()->GetLineNum();
		int pointIdx = pDoc->m_Data.GetCurLine()->GetCurPointIndex();
		SPoint pt = pDoc->m_Data.GetLine(lineIdx)->GetPoint(pointIdx);
		pt.m_x += dx;
		pt.m_y += dy;
		pDoc->m_Data.GetLine(lineIdx)->SetPoint(pointIdx, pt);

		pDoc->SetModifiedFlag(TRUE);
		Invalidate();
	}
	else
	{
		// 无选中对象或不在选择模式 -> 移动视图
		pDoc->m_StateShow.m_dx += dx * pDoc->m_StateShow.m_r;
		pDoc->m_StateShow.m_dy += dy * pDoc->m_StateShow.m_r;
		Invalidate();
	}
}

SPoint CDrawCADView::ConstrainAngle(const SPoint& start, const SPoint& current, int& outDirIndex, double& outAngleDeg) const
{
	SPoint vec = { current.m_x - start.m_x, current.m_y - start.m_y };
	double len = sqrt(vec.m_x * vec.m_x + vec.m_y * vec.m_y);
	if (len < 1e-6)
	{
		outDirIndex = -1;
		outAngleDeg = 0.0;
		return current;
	}

	// 寻找点积绝对值最大的方向
	double maxAbsDot = -1e100;
	int bestIdx = -1;
	double bestDot = 0.0; // 原始点积（带符号），用于投影
	for (int i = 0; i < 8; ++i)
	{
		double dot = vec.m_x * s_angleDirs[i].m_x + vec.m_y * s_angleDirs[i].m_y;
		double absDot = fabs(dot);
		if (absDot > maxAbsDot)
		{
			maxAbsDot = absDot;
			bestIdx = i;
			bestDot = dot; // 记录原始点积（可能为负）
		}
	}

	// 计算最小夹角（0~90°）
	double cosAngle = maxAbsDot / len;
	double angleRad = acos(cosAngle);          // 结果在 [0, π/2]
	double angleDeg = angleRad * 180.0 / 3.141592653589793;

	outDirIndex = bestIdx;
	outAngleDeg = angleDeg;

	// 投影点（使用带符号的点积，以保证投影点在正确方向）
	SPoint result;
	result.m_x = start.m_x + bestDot * s_angleDirs[bestIdx].m_x;
	result.m_y = start.m_y + bestDot * s_angleDirs[bestIdx].m_y;
	return result;
}

void CDrawCADView::UpdateRubberBandEnd(CPoint screenPt)
{
	CDrawCADDoc* pDoc = GetDocument();
	if (!pDoc || !m_bRubberBanding) return;

	// 将屏幕点转换为世界坐标
	SPoint worldPt;
	worldPt.m_x = (screenPt.x - pDoc->m_StateShow.m_dx) / pDoc->m_StateShow.m_r;
	worldPt.m_y = -(screenPt.y + pDoc->m_StateShow.m_dy) / pDoc->m_StateShow.m_r;

	// 角度约束仅在启用且当前没有吸附高亮时生效
	if (m_bAngleSnapEnabled && !m_bHighlightPoint)
	{
		int dirIdx;
		double angleDeg;
		SPoint constrainedPt = ConstrainAngle(m_rubberStartWorld, worldPt, dirIdx, angleDeg);

		const double THRESHOLD = 20.0; // 可根据需要调整
		if (angleDeg <= THRESHOLD)
		{
			m_rubberEndWorld = constrainedPt;
			m_bestAngleDirIndex = dirIdx;

			// 判断投影点位于正向还是反向
			double dx = constrainedPt.m_x - m_rubberStartWorld.m_x;
			double dy = constrainedPt.m_y - m_rubberStartWorld.m_y;
			m_bestAngleDirPositive = (dx * s_angleDirs[dirIdx].m_x + dy * s_angleDirs[dirIdx].m_y) > 0;
		}
		else
		{
			m_rubberEndWorld = worldPt;
			m_bestAngleDirIndex = -1;
		}
	}
	else
	{
		m_rubberEndWorld = worldPt;
		m_bestAngleDirIndex = -1;
	}

	Invalidate();
}

void CDrawCADView::RefreshRubberBand()
{
	if (!m_bRubberBanding) return;
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	pt = SnapToPoint(pt);          // 先进行吸附处理
	UpdateRubberBandEnd(pt);       // 再更新终点
}

void CDrawCADView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (nChar == VK_SHIFT)
	{
		if (m_bAngleSnapEnabled)
		{
			m_bAngleSnapEnabled = false;
			RefreshRubberBand();
		}
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}
