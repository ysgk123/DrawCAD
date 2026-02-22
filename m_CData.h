#pragma once
#include <vector>
#include "m_CLine.h"
#define MAX_LINE_NUM 1024// 最大线段数量
#define MAX_SCALE 100.0
#define MIN_SCALE 0.05
#define MAX_UNDO_STEPS 20

class m_CData
{
	m_CLine* m_Line[MAX_LINE_NUM];	// 线段数组
	m_CLine* m_CurLine;				// 线段数组
	int m_LineSum;					// 线段数量
	CRect m_rcDraw;					// 绘图区域
public:
	m_CData();
	~m_CData();
	void Clear();
	
	bool AddLine(SPoint p0);  // 返回 true 表示成功，false 表示失败（已达上限）													//添加线段
	void AddPoint(SPoint p0, int sn = 0);										//添加节点
	
	void ShowLine(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state);	//显示线段
	void ShowCurLine(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state);		//高亮显示当前线段
	void ShowPoint(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state, int d = 3); //显示所有节点
	void ShowCurPoint(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state, int d = 10); //显示所有节点
	
	void EndLine() { m_CurLine = nullptr; };									//结束线段
	bool DelLine();																//删除当前线段
	bool DelPoint();															//删除当前节点
	
	bool SetCurLine(SPoint p0, double d = 5);										//选择当前曲线，节点

	bool SavePLT(const CString& filePath);		// 保存为PLT文件
	bool LoadPLT(const CString& filePath);		// 加载PLT文件

	bool SaveDXF(const CString& filePath);		// 保存为 DXF 文件
	bool LoadDXF(const CString& filePath);		// 加载 DXF 文件

	bool InsertPLT(const CString& filePath);   // 插入 PLT 文件
	bool InsertDXF(const CString& filePath);   // 插入 DXF 文件
	
	void GetRect();			//获取绘图范围
	CPoint GetCenter() { return m_rcDraw.CenterPoint(); }	//取得图形中心
	CRect GetmRect() { GetRect(); return m_rcDraw; }		//取得绘图范围
	int GetLineSum(){ return m_LineSum; }					//获取线段数量		

	m_CData(const m_CData& other);             // 拷贝构造
	m_CData& operator=(const m_CData& other);  // 赋值操作符

	// 获取指定索引的线条指针（索引从0开始）
	m_CLine* GetLine(int idx) const;

	m_CLine* GetCurLine() const { return m_CurLine; }

	// 根据索引设置当前线条（不选中任何节点）
	void SetCurLineByIndex(int idx);
	// 查找距离世界坐标点 pWorld 最近的节点，阈值 thresholdWorld（世界单位）
	// 成功返回 true，并填充 lineIdx 和 pointIdx
	bool FindNearestPoint(const SPoint& pWorld, double thresholdWorld, int& lineIdx, int& pointIdx);

	// 查找距离世界坐标点 pWorld 最近的线段，阈值 thresholdWorld
	// 成功返回 true，并填充 lineIdx、segIdx（线段起点索引），以及垂足点 footWorld
	bool FindNearestLine(const SPoint& pWorld, double thresholdWorld, int& lineIdx, int& segIdx, SPoint& footWorld);

	//复制粘贴
public:
	// 选择集管理
	void AddSelected(int idx);
	void RemoveSelected(int idx);
	void ClearSelected();
	bool IsSelected(int idx) const;
	int GetSelectedCount() const;
	std::vector<int> GetSelectedIndices() const;
	void SelectAll();

	// 剪贴板操作
	void CopySelectedToClipboard();
	void CutSelectedToClipboard();
	void PasteFromClipboard(const SPoint* pTargetWorld = nullptr);

	static void SetClipboardData(const m_CData& data);
	static bool HasClipboardData();
	static void ClearClipboard();
	static m_CData GetClipboardData();  // 获取副本用于粘贴

	//批量删除线
	int DeleteSelectedLines();   // 返回删除的线条数量
private:
	std::vector<int> m_selectedIndices;   // 当前选中的线条索引
	static m_CData s_clipboardData;  // 内部剪贴板数据
};

