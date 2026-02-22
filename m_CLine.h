#pragma once

#define MAX_POINT_NUM 1024

struct SState {                                     //显示状态
    double m_r;                                     //放大倍率
    double    m_dx;                                    //水平移动
    double    m_dy;                                    //垂直移动
    bool   m_bViewPoint;                            //是否显示节点
};

struct SPoint
{
	double m_x, m_y;
	int m_PointNum;
    SPoint(double x = 0, double y = 0,int num = 0):m_x(x), m_y(y), m_PointNum(num){}
    static void Getxy(SPoint& p, SPoint& Worigin, SPoint& Sorigin, SState state)      //point到屏幕坐标的转换(p是世界坐标,origin是世界坐标原点,state为显示状态)
    {//世界转屏幕坐标
        // 基于原始原点平移（消除原点偏移）
        double temp_x = p.m_x - Worigin.m_x;
        double temp_y = Worigin.m_y - p.m_y;  // 反转y轴，防止图形翻转

        // 缩放（基于原始缩放因子，不修改原始坐标）
        p.m_x = temp_x * state.m_r + Sorigin.m_x;
        p.m_y = temp_y * state.m_r + Sorigin.m_y;
    }
    static void GetXY(SPoint& p, SPoint& Worigin, SPoint& Sorigin,SState state)
    {//屏幕转世界坐标
        // 反向缩放（先缩放，再平移，与世界转屏幕逆序）
        double temp_x = (p.m_x - Sorigin.m_x) / state.m_r;
        double temp_y = (p.m_y - Sorigin.m_y) / state.m_r;

        // 反向平移（恢复原点偏移）
        p.m_x = temp_x + Worigin.m_x;
        p.m_y = Worigin.m_y - temp_y;  // 反转y轴回退
    }
};//默认屏幕坐标为(0,0)

enum DrawingMode
{
    LineMode,
    SelectMode,
    /*PointMode,
    RectMode,
    EllipseMode,
    FreeMode,
    TextMode*/
};

enum DragMode 
{   
    None, 
    DragPoint, 
    DragLine,
    DragMultiLine
};

class m_CLine
{
    int m_LineNum;
    int m_PointSum;
    SPoint* m_Points,*m_CurPoint;
public:
    
    m_CLine(SPoint pPoints,int num = 0);
    ~m_CLine();
    void AddPoint(SPoint pPoint);
    bool DelPoint();
    
    bool SetCurPoint(SPoint p0, double d = 5.0);

    void ShowCurPoint(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state, int d = 10);
    void ShowAllPoint(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state, int d = 3);
    void ShowLine(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state);

    int GetNum() { return m_PointSum; }
    int GetLineNum() { return m_LineNum; }
    void SetLineNum(int i) { m_LineNum = i; }

    SPoint GetPoint(int i) { return m_Points[i]; }

public:
    m_CLine(const m_CLine& other);            // 拷贝构造
    m_CLine& operator=(const m_CLine& other); // 赋值操作符

    //移动功能 
    // 修改指定索引的节点坐标（同时更新序号，但序号通常不变）
    void SetPoint(int idx, const SPoint& pt);

    // 清除当前节点指针（设为 nullptr）
    void ClearCurPoint();

    // 在指定索引之后插入一个节点（新节点成为 idx+1）
    bool InsertPoint(int afterIndex, const SPoint& pt);

//复制粘贴
public:
    m_CLine();   // 默认构造函数
    // 序列化：将当前线条数据写入 archive（用于剪贴板或文件）
    void Serialize(CArchive& ar);

    // 静态方法：从 archive 读取一条线并返回新创建的 m_CLine 对象
    static m_CLine* CreateFromArchive(CArchive& ar);
public:
    int GetCurPointIndex() const { return m_CurPoint ? m_CurPoint->m_PointNum : -1; }   //吸附
};

