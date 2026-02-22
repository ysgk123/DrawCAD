#include "pch.h"
#include "m_CLine.h"
m_CLine::m_CLine(SPoint pPoints,int num)
{
	m_PointSum = 0;
	m_LineNum = num;						//设置线序号
	m_Points = new SPoint[MAX_POINT_NUM];	// 最大点数数组
	m_Points[m_PointSum] = pPoints;
	m_Points[m_PointSum].m_PointNum = m_PointSum;
	m_CurPoint = &m_Points[m_PointSum];		// 指向数组元素
	m_PointSum++;
}

m_CLine::~m_CLine()
{
	delete[]m_Points;
}

void m_CLine::AddPoint(SPoint pPoint)
{
	if (m_PointSum >= MAX_POINT_NUM)
	{
		AfxMessageBox(_T("点数量已达上限！"));
		return;
	}
	m_Points[m_PointSum] = pPoint;
	m_Points[m_PointSum].m_PointNum = m_PointSum; // 设置数组中的序号
	m_CurPoint = &m_Points[m_PointSum];  // 指向数组元素
	m_PointSum++;
}

bool m_CLine::DelPoint()
{
	if (m_PointSum <= 0 || m_CurPoint == NULL)
	{
		return false;
	}
	
	for (int i = m_CurPoint->m_PointNum; i < m_PointSum - 1; i++)
	{
		m_Points[i] = m_Points[i + 1];			// 前移
		m_Points[i].m_PointNum = i;				// 更新序号（保持连续）
	}
	m_PointSum--;								// 减少点数
	m_CurPoint = nullptr;						// 重置当前点序号
	return true;
}

bool m_CLine::SetCurPoint(SPoint p0, double d)
{
	for (int i = 0; i < m_PointSum; i++) {	//在圆形范围内查找
		// 判断节点是否在指定范围内
		double dx = p0.m_x - m_Points[i].m_x;
		double dy = p0.m_y - m_Points[i].m_y;
		if (dx * dx + dy * dy <= d * d)
		{
			m_CurPoint = &m_Points[i];		// 设置当前节点为选中的节点
			return true;					// 选中成功
		}
	}
	return false;							// 未找到合适的节点
}

void m_CLine::ShowCurPoint(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state, int d)
{
	if (!m_CurPoint)
	{
		return;
	}
	// 绘制当前节点
	SPoint p = *m_CurPoint;
	m_CurPoint->Getxy(p,Worigin,Sorigin,state);
	pDC->Rectangle(
		p.m_x + state.m_dx - d,  // 左边界
		p.m_y - state.m_dy - d,  // 上边界
		p.m_x + state.m_dx + d,  // 右边界
		p.m_y - state.m_dy + d   // 下边界
	);
}

void m_CLine::ShowAllPoint(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state, int d)
{
	SPoint temp;
	for (int i = 0; i < m_PointSum; i++)
	{
		temp = m_Points[i];
		temp.Getxy(temp, Worigin, Sorigin, state);
		pDC->Rectangle(
			temp.m_x + state.m_dx - d,  // 左边界
			temp.m_y - state.m_dy - d,  // 上边界
			temp.m_x + state.m_dx + d,  // 右边界
			temp.m_y - state.m_dy + d   // 下边界
		);
	}
}

void m_CLine::ShowLine(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state)
{
	if (m_PointSum >= 2)                     // 至少需要两个节点才能绘制线条
	{
		SPoint tPoint = m_Points[0];        // 获取第一个节点
		SPoint::Getxy(tPoint, Worigin, Sorigin, state);  // 转换坐标
		pDC->MoveTo(tPoint.m_x + state.m_dx, tPoint.m_y - state.m_dy); // 移动到起始节点
		for (int i = 1; i < m_PointSum; i++) {
			tPoint = m_Points[i];           // 获取下一个节点
			SPoint::Getxy(tPoint, Worigin, Sorigin, state); // 转换坐标
			pDC->LineTo(tPoint.m_x + state.m_dx, tPoint.m_y - state.m_dy);//绘制线到该节点
		}
	}
}

m_CLine::m_CLine(const m_CLine& other)
{
	m_PointSum = other.m_PointSum;
	m_LineNum = other.m_LineNum;
	m_Points = new SPoint[MAX_POINT_NUM];
	for (int i = 0; i < m_PointSum; i++)
		m_Points[i] = other.m_Points[i];

	if (other.m_CurPoint)
	{
		int idx = other.m_CurPoint - other.m_Points;
		m_CurPoint = &m_Points[idx];
	}
	else
		m_CurPoint = nullptr;
}

m_CLine& m_CLine::operator=(const m_CLine& other)
{
	if (this != &other)
	{
		delete[] m_Points;
		m_PointSum = other.m_PointSum;
		m_LineNum = other.m_LineNum;
		m_Points = new SPoint[MAX_POINT_NUM];
		for (int i = 0; i < m_PointSum; i++)
			m_Points[i] = other.m_Points[i];

		if (other.m_CurPoint)
		{
			int idx = other.m_CurPoint - other.m_Points;
			m_CurPoint = &m_Points[idx];
		}
		else
			m_CurPoint = nullptr;
	}
	return *this;
}

bool m_CLine::InsertPoint(int afterIndex, const SPoint& pt)
{
	if (m_PointSum >= MAX_POINT_NUM || afterIndex < 0 || afterIndex >= m_PointSum)
		return false;

	// 从后向前移动元素
	for (int i = m_PointSum; i > afterIndex + 1; i--)
	{
		m_Points[i] = m_Points[i - 1];
		m_Points[i].m_PointNum = i;
	}
	m_Points[afterIndex + 1] = pt;
	m_Points[afterIndex + 1].m_PointNum = afterIndex + 1;
	m_PointSum++;
	return true;
}

void m_CLine::SetPoint(int idx, const SPoint& pt)
{
	if (idx >= 0 && idx < m_PointSum)
	{
		m_Points[idx] = pt;
		m_Points[idx].m_PointNum = idx;  // 确保序号一致
		// 如果当前节点正好是修改的点，需要更新 m_CurPoint 指向新地址？不需要，因为地址未变
	}
}

void m_CLine::ClearCurPoint()
{
	m_CurPoint = nullptr;
}

m_CLine::m_CLine()
{
	m_Points = new SPoint[MAX_POINT_NUM];
	m_PointSum = 0;
	m_CurPoint = nullptr;
	m_LineNum = 0;   // 临时编号，后面会重新设置
}

// 序列化：写入或读取线条数据
void m_CLine::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// 写入部分不变
		ar << m_PointSum;
		for (int i = 0; i < m_PointSum; i++)
		{
			ar << m_Points[i].m_x;
			ar << m_Points[i].m_y;
		}
	}
	else
	{
		int n;
		ar >> n;
		if (n > 0)
		{
			// 检查点数是否超过上限
			if (n > MAX_POINT_NUM)
			{
				AfxMessageBox(_T("警告：剪贴板中某线条点数超过上限，将被截断！"));
				n = MAX_POINT_NUM;   // 截断，避免越界
			}
			delete[] m_Points;
			m_PointSum = n;
			m_Points = new SPoint[MAX_POINT_NUM]; // 仍按最大分配
			for (int i = 0; i < n; i++)
			{
				double x, y;
				ar >> x >> y;
				m_Points[i] = SPoint(x, y, i);
			}
			m_CurPoint = nullptr;
		}
	}
}

m_CLine* m_CLine::CreateFromArchive(CArchive& ar)
{
	m_CLine* pLine = new m_CLine();   // 默认构造函数已分配 m_Points
	pLine->Serialize(ar);             // 填充数据（已检查点数）
	return pLine;
}