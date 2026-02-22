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

// DrawCADView.h: CDrawCADView 类的接口
//

#pragma once
#include "DrawCADDoc.h"
#include "m_CLine.h"
#include <map>


class CDrawCADView : public CView
{
protected: // 仅从序列化创建
	CDrawCADView() noexcept;
	DECLARE_DYNCREATE(CDrawCADView)

// 特性
public:
	CDrawCADDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CDrawCADView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	void Draw(CDC* pDC);
	void MoveLine(double x, double y);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg void OnLinecolor();
	afx_msg void OnCurlinecolor();
	afx_msg void OnBackcolor();
	afx_msg void OnDelpoint();
	afx_msg void OnUpdateDelpoint(CCmdUI* pCmdUI);
	afx_msg void OnDelline();
	afx_msg void OnUpdateDelline(CCmdUI* pCmdUI);
	afx_msg void OnAdapt();
	void CenterView();
	void ZoomAtPoint(CPoint point, double factor);// 在特定点放大
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
private:
	bool m_bDragging;				// 是否正在中键拖拽
	CPoint m_dragStart;				// 拖拽起始点（客户区坐标）
	double m_dragStartDx;			// 拖拽开始时的 dx
	double m_dragStartDy;			// 拖拽开始时的 dy
	CPoint m_lastZoomPoint;			// 保存最后一次滚轮缩放时的鼠标客户区坐标
	bool   m_bHasZoomPoint;			// 标记是否已保存有效点
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CPoint GetLastZoomPoint() const { return m_lastZoomPoint; }
	bool HasLastZoomPoint() const { return m_bHasZoomPoint; }
private:
	DragMode m_dragMode;			// 当前拖拽模式
	CPoint   m_dragStartScreen;     // 拖拽起始屏幕坐标
	SPoint   m_dragStartWorld;      // 拖拽起始时被拖拽点的世界坐标（拖点用）

	// 拖线时记录整条线的原始点坐标，或者直接计算位移增量		移动
	std::vector<SPoint> m_lineOrigPoints;  // 拖线时暂存原始点（可选）
	int      m_dragLineIdx;         // 被拖拽的曲线索引（拖线用）
	int      m_dragPointIdx;        // 被拖拽的节点索引（拖点用）

	//橡皮筋
	bool m_bRubberBanding;          // 是否正在橡皮筋（临时线预览）
	SPoint m_rubberStartWorld;      // 橡皮筋起点（世界坐标）
	SPoint m_rubberEndWorld;        // 橡皮筋终点（世界坐标，随鼠标移动更新）
	SPoint m_dragStartMouseWorld;   // 拖拽开始时鼠标的世界坐标

	//吸附高亮
	bool m_bHighlightPoint;                     // 是否有高亮点
	int m_highlightLineIdx;                     // 高亮点所在线条索引
	int m_highlightPointIdx;                    // 高亮点在线上索引
	CPoint SnapToPoint(CPoint point);			// 吸附函数声明

	bool GetMouseWorldPoint(CPoint& screenPt, SPoint& worldPt) const;
public:
	void ClearHighlight();                      // 清除吸附高亮
	int SNAP_TOLERANCE = 10;					// 吸附阈值（像素）

public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	void ClearRubberBand();  // 供框架调用以清除橡皮筋状态

	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) override;

	void UpdateSelection(CPoint point, bool bCtrlPressed);	//支持 Ctrl+点击多选
	std::map<int, std::vector<SPoint>> m_selectedOrigPoints;
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);	//esc取消选择

	void MoveByStep(double dx, double dy);   // dx, dy 为世界坐标步长（毫米）
private:
	bool m_bAngleSnapEnabled;			// 是否启用角度限制（Shift按下）
	int  m_bestAngleDirIndex;			// 当前捕捉到的方向索引，-1表示无
	static const SPoint s_angleDirs[8]; // 预设的8个方向单位向量

	SPoint ConstrainAngle(const SPoint& start, const SPoint& current, int& outDirIndex, double& outAngleDeg) const;
	void UpdateRubberBandEnd(CPoint screenPt);   // 根据屏幕点更新橡皮筋终点
	void RefreshRubberBand();                    // 用当前鼠标位置刷新橡皮筋（用于Shift按下/释放时）
	bool m_bestAngleDirPositive;  // true: 投影点在正向，false: 在反向
public:
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG  // DrawCADView.cpp 中的调试版本
inline CDrawCADDoc* CDrawCADView::GetDocument() const
   { return reinterpret_cast<CDrawCADDoc*>(m_pDocument); }
#endif

