#include "pch.h"
#include "m_CData.h"
#include <algorithm> 
#include <afx.h>
#include <afxtempl.h>
m_CData m_CData::s_clipboardData;

m_CData::m_CData()
{
    m_LineSum = 0; // 线数量
    m_CurLine = nullptr; // 当前曲线指针
}
m_CData::~m_CData()
{
    Clear(); // 清空数据
}
void m_CData::Clear() 
{// 清空数据
    for (int i = 0; i < m_LineSum; i++)
        delete m_Line[i]; // 释放每条曲线的内存
    m_LineSum = 0; // 重置线数量
    m_CurLine = nullptr; // 清空当前曲线指针
}

void m_CData::ShowLine(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state)
{
    if (m_LineSum > 0)
    {
        for (int i = 0; i < m_LineSum; i++)
            m_Line[i]->ShowLine(pDC, Worigin, Sorigin, state); // 显示每条曲线
    }
}

void m_CData::ShowCurLine(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state)
{
    if (m_CurLine)
        m_CurLine->ShowLine(pDC, Worigin, Sorigin, state); // 显示当前曲线
}

void m_CData::ShowPoint(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state, int d)
{
    if (m_LineSum > 0)
    {
        for (int i = 0; i < m_LineSum; i++)
        {
            m_Line[i]->ShowAllPoint(pDC, Worigin, Sorigin, state, d); // 显示每条曲线的节点
        }
    }
}

void m_CData::ShowCurPoint(CDC* pDC, SPoint& Worigin, SPoint& Sorigin, SState state, int d)
{
    if (m_CurLine)
    {
        m_CurLine->ShowCurPoint(pDC, Worigin, Sorigin, state, d); // 显示当前曲线的节点
    }
}

bool m_CData::AddLine(SPoint p0)
{
    if (m_LineSum >= MAX_LINE_NUM) {
        AfxMessageBox(_T("线条数量已达上限！"));
        return false;
    }
    m_Line[m_LineSum] = new m_CLine(p0,m_LineSum);  // 新建曲线
    m_Line[m_LineSum]->SetLineNum(m_LineSum);       // 设置曲线序号
    m_LineSum++;                                    // 线数量加1
    m_CurLine = m_Line[m_LineSum - 1];              // 设置当前曲线指针
    return true;
}

void m_CData::AddPoint(SPoint p0, int sn) {         // 为指定曲线添加一个节点，返回节点序号
    if (m_LineSum >= MAX_LINE_NUM) {
        AfxMessageBox(_T("线条数量已达上限！"));
        return;
    }
    if (sn)                                         // 如果指定了线序号
        m_Line[sn - 1]->AddPoint(p0);               // 给指定曲线添加节点
    else if (m_CurLine) {                           // 如果当前线存在
        m_CurLine->AddPoint(p0);                    // 给当前曲线添加节点
    }
    else {                                          // 如果没有指定曲线和当前曲线，则添加新曲线
        AddLine(p0);                                // 添加新曲线
    }
}

bool m_CData::DelLine()
{
    if (m_CurLine)
    {
        int index = m_CurLine->GetLineNum();    // 保存当前曲线序号
        delete m_CurLine;                       // 删除当前曲线

        // 更新选择集：移除被删索引，并将大于 index 的索引减 1
        std::vector<int> newSelected;
        for (int selIdx : m_selectedIndices)
        {
            if (selIdx == index)
                continue;  // 被删除的线条如果被选中，直接丢弃
            else if (selIdx > index)
                newSelected.push_back(selIdx - 1);  // 大于的被删索引减1
            else // selIdx < index
                newSelected.push_back(selIdx);      // 小于的不变
        }
        m_selectedIndices = newSelected;

        // 后续线条前移
        for (int i = index; i < m_LineSum - 1; i++)
        {
            m_Line[i] = m_Line[i + 1];          // 后面的曲线前移
            m_Line[i]->SetLineNum(i);           // 更新曲线序号
        }
        m_LineSum--;                            // 线条总数减1
        m_CurLine = nullptr;                    // 清空当前线条
        return true;
    }
    return false;
}

bool m_CData::DelPoint()
{
    if (m_CurLine)
    {
        m_CurLine->DelPoint();                  // 删除当前曲线的当前节点
        return true;
    }
    return false;
}

bool m_CData::SetCurLine(SPoint p0, double d)
{
    for (int i = 0; i < m_LineSum; i++)
    {
        if (m_Line[i]->SetCurPoint(p0, d)) // 找到第一个有节点的曲线
        {
            m_CurLine = m_Line[i]; // 设置当前曲线指针
            return true; // 成功
        }
    }
    return false; // 没有找到
}

bool m_CData::SavePLT(const CString& filePath)
{
    CStdioFile file;
    try
    {
        if (!file.Open(filePath, CFile::modeWrite | CFile::modeCreate | CFile::typeText))
        {
            AfxMessageBox(_T("无法创建PLT文件！"));
            return false;
        }

        // 写入头部指令
        file.WriteString(_T("IN;\r\n"));
        file.WriteString(_T("SP1;\r\n"));

        for (int i = 0; i < m_LineSum; i++)
        {
            m_CLine* pLine = m_Line[i];
            int pointCount = pLine->GetNum();
            if (pointCount == 0) continue;

            SPoint p0 = pLine->GetPoint(0);
            CString strCmd;
            strCmd.Format(_T("G00X%.2fY%.2f;\r\n"), p0.m_x, p0.m_y);
            file.WriteString(strCmd);
            file.WriteString(_T("PD;\r\n"));

            for (int j = 1; j < pointCount; j++)
            {
                SPoint p = pLine->GetPoint(j);
                strCmd.Format(_T("G01X%.2fY%.2f;\r\n"), p.m_x, p.m_y);
                file.WriteString(strCmd);
            }
            file.WriteString(_T("PU;\r\n"));
        }

        file.WriteString(_T("EM;\r\n"));
        file.Close();
        return true;
    }
    catch (CFileException* e)
    {
        e->Delete();
        AfxMessageBox(_T("保存PLT文件时发生错误！"));
        return false;
    }
}

bool m_CData::LoadPLT(const CString& filePath)
{
    m_CData tempData;                     // 临时对象
    CStdioFile file;
    try
    {
        if (!file.Open(filePath, CFile::modeRead | CFile::typeText))
        {
            AfxMessageBox(_T("打开PLT文件失败！"));
            return false;
        }

        CString strLine;
        SPoint curPoint;
        bool bPenDown = false;
        m_CLine* pCurLine = nullptr;

        while (file.ReadString(strLine))
        {
            strLine.Trim();
            if (strLine.IsEmpty()) continue;

            if (strLine.CompareNoCase(_T("PU;")) == 0)
            {
                bPenDown = false;
                pCurLine = nullptr;
                continue;
            }
            if (strLine.CompareNoCase(_T("PD;")) == 0)
            {
                bPenDown = true;
                continue;
            }

            if (strLine.Left(3) == _T("G00") || strLine.Left(3) == _T("G01"))
            {
                double x = 0.0, y = 0.0;
                int nXPos = strLine.Find(_T('X'));
                int nYPos = strLine.Find(_T('Y'));
                if (nXPos != -1 && nYPos != -1)
                {
                    CString strX = strLine.Mid(nXPos + 1, nYPos - nXPos - 1);
                    CString strY = strLine.Mid(nYPos + 1);
                    int nSemi = strY.Find(_T(';'));
                    if (nSemi != -1) strY = strY.Left(nSemi);
                    x = _ttof(strX);
                    y = _ttof(strY);
                    curPoint = SPoint(x, y, 0);
                }

                if (strLine.Left(3) == _T("G00"))
                {
                    if (!tempData.AddLine(curPoint))   // 操作临时对象
                    {
                        file.Close();
                        return false;                   // 失败直接返回，tempData 析构自动清理
                    }
                    pCurLine = tempData.GetCurLine();   // 记录当前线
                    bPenDown = false;
                }
                else if (strLine.Left(3) == _T("G01") && bPenDown && pCurLine)
                {
                    pCurLine->AddPoint(curPoint);       // 直接向当前线添加点
                }
            }
        }
        file.Close();

        // 全部成功，用临时对象替换当前对象
        *this = tempData;
        GetRect();          // 重新计算包围盒
        ClearSelected();    // 新加载的文档不应有选中项
        return true;
    }
    catch (CFileException* e)
    {
        e->Delete();
        AfxMessageBox(_T("加载PLT文件时发生错误！"));
        return false;
    }
}

// 保存 DXF 文件（仅输出 LWPOLYLINE 实体）
bool m_CData::SaveDXF(const CString& filePath)
{
    CStdioFile file;
    CFileException ex;
    try
    {
        if (!file.Open(filePath, CFile::modeWrite | CFile::modeCreate | CFile::typeText))
        {
            AfxMessageBox(_T("无法创建DXF文件！"));
            return false;
        }

        const TCHAR* sCRLF = _T("\r\n");

        // ==================== HEADER 段 ====================
        file.WriteString(_T("0") + CString(sCRLF));
        file.WriteString(_T("SECTION") + CString(sCRLF));
        file.WriteString(_T("2") + CString(sCRLF));
        file.WriteString(_T("HEADER") + CString(sCRLF));

        // 必要的 HEADER 变量
        file.WriteString(_T("9") + CString(sCRLF));
        file.WriteString(_T("$ACADVER") + CString(sCRLF));
        file.WriteString(_T("1") + CString(sCRLF));
        file.WriteString(_T("AC1009") + CString(sCRLF));  // 兼容 R12 版本

        file.WriteString(_T("9") + CString(sCRLF));
        file.WriteString(_T("$INSBASE") + CString(sCRLF));
        file.WriteString(_T("10") + CString(sCRLF));
        file.WriteString(_T("0.0") + CString(sCRLF));
        file.WriteString(_T("20") + CString(sCRLF));
        file.WriteString(_T("0.0") + CString(sCRLF));
        file.WriteString(_T("30") + CString(sCRLF));
        file.WriteString(_T("0.0") + CString(sCRLF));

        // 计算图形范围用于 EXTMIN/EXTMAX
        GetRect();  // 确保 m_rcDraw 是最新的
        CRect rect = m_rcDraw;
        rect.NormalizeRect();

        file.WriteString(_T("9") + CString(sCRLF));
        file.WriteString(_T("$EXTMIN") + CString(sCRLF));
        file.WriteString(_T("10") + CString(sCRLF));
        CString str;
        str.Format(_T("%.3f") + CString(sCRLF), rect.left);
        file.WriteString(str);
        file.WriteString(_T("20") + CString(sCRLF));
        str.Format(_T("%.3f") + CString(sCRLF), rect.bottom);
        file.WriteString(str);
        file.WriteString(_T("30") + CString(sCRLF));
        file.WriteString(_T("0.0") + CString(sCRLF));

        file.WriteString(_T("9") + CString(sCRLF));
        file.WriteString(_T("$EXTMAX") + CString(sCRLF));
        file.WriteString(_T("10") + CString(sCRLF));
        str.Format(_T("%.3f") + CString(sCRLF), rect.right);
        file.WriteString(str);
        file.WriteString(_T("20") + CString(sCRLF));
        str.Format(_T("%.3f") + CString(sCRLF), rect.top);
        file.WriteString(str);
        file.WriteString(_T("30") + CString(sCRLF));
        file.WriteString(_T("0.0") + CString(sCRLF));

        file.WriteString(_T("0") + CString(sCRLF));
        file.WriteString(_T("ENDSEC") + CString(sCRLF));

        // ==================== TABLES 段 ====================
        file.WriteString(_T("0") + CString(sCRLF));
        file.WriteString(_T("SECTION") + CString(sCRLF));
        file.WriteString(_T("2") + CString(sCRLF));
        file.WriteString(_T("TABLES") + CString(sCRLF));

        // LAYER 表
        file.WriteString(_T("0") + CString(sCRLF));
        file.WriteString(_T("TABLE") + CString(sCRLF));
        file.WriteString(_T("2") + CString(sCRLF));
        file.WriteString(_T("LAYER") + CString(sCRLF));
        file.WriteString(_T("70") + CString(sCRLF));
        file.WriteString(_T("1") + CString(sCRLF));  // 表记录数（至少1个）

        // 定义图层 0
        file.WriteString(_T("0") + CString(sCRLF));
        file.WriteString(_T("LAYER") + CString(sCRLF));
        file.WriteString(_T("2") + CString(sCRLF));
        file.WriteString(_T("0") + CString(sCRLF));      // 图层名
        file.WriteString(_T("70") + CString(sCRLF));
        file.WriteString(_T("0") + CString(sCRLF));      // 标志
        file.WriteString(_T("62") + CString(sCRLF));
        file.WriteString(_T("7") + CString(sCRLF));      // 颜色号（7=白色/黑色）
        file.WriteString(_T("6") + CString(sCRLF));
        file.WriteString(_T("CONTINUOUS") + CString(sCRLF)); // 线型

        // 结束 LAYER 表
        file.WriteString(_T("0") + CString(sCRLF));
        file.WriteString(_T("ENDTAB") + CString(sCRLF));

        file.WriteString(_T("0") + CString(sCRLF));
        file.WriteString(_T("ENDSEC") + CString(sCRLF));

        // ==================== ENTITIES 段 ====================
        file.WriteString(_T("0") + CString(sCRLF));
        file.WriteString(_T("SECTION") + CString(sCRLF));
        file.WriteString(_T("2") + CString(sCRLF));
        file.WriteString(_T("ENTITIES") + CString(sCRLF));

        for (int i = 0; i < m_LineSum; i++)
        {
            m_CLine* pLine = m_Line[i];
            int pointCount = pLine->GetNum();
            if (pointCount < 2) continue;

            // 开始 LWPOLYLINE
            file.WriteString(_T("0") + CString(sCRLF));
            file.WriteString(_T("LWPOLYLINE") + CString(sCRLF));
            file.WriteString(_T("5") + CString(sCRLF));
            file.WriteString(_T("0") + CString(sCRLF));   // 句柄（可任意）
            file.WriteString(_T("100") + CString(sCRLF));
            file.WriteString(_T("AcDbEntity") + CString(sCRLF));
            file.WriteString(_T("8") + CString(sCRLF));
            file.WriteString(_T("0") + CString(sCRLF));   // 图层名
            file.WriteString(_T("100") + CString(sCRLF));
            file.WriteString(_T("AcDbPolyline") + CString(sCRLF));
            file.WriteString(_T("90") + CString(sCRLF));
            CString str;
            str.Format(_T("%d") + CString(sCRLF), pointCount);
            file.WriteString(str);
            file.WriteString(_T("70") + CString(sCRLF));
            file.WriteString(_T("0") + CString(sCRLF));   // 0=不闭合

            // 输出顶点坐标
            for (int j = 0; j < pointCount; j++)
            {
                SPoint pt = pLine->GetPoint(j);
                file.WriteString(_T("10") + CString(sCRLF));
                str.Format(_T("%.3f") + CString(sCRLF), pt.m_x);
                file.WriteString(str);
                file.WriteString(_T("20") + CString(sCRLF));
                str.Format(_T("%.3f") + CString(sCRLF), pt.m_y);
                file.WriteString(str);
            }
        }

        file.WriteString(_T("0") + CString(sCRLF));
        file.WriteString(_T("ENDSEC") + CString(sCRLF));
        file.WriteString(_T("0") + CString(sCRLF));
        file.WriteString(_T("EOF") + CString(sCRLF));

        file.Close();
        return true;
    }
    catch (CFileException* e)
    {
        e->Delete();
        AfxMessageBox(_T("保存DXF文件时发生错误！"));
        return false;
    }
}

// 加载 DXF 文件（仅解析 LWPOLYLINE 实体）
bool m_CData::LoadDXF(const CString& filePath)
{
    m_CData tempData;                     // 临时对象
    CStdioFile file;
    try
    {
        if (!file.Open(filePath, CFile::modeRead | CFile::typeText))
        {
            AfxMessageBox(_T("打开DXF文件失败！"));
            return false;
        }

        CString strLine;
        bool bInEntities = false;
        bool bInPolyline = false;
        bool bInLine = false;              // 新增：正在解析 LINE 实体
        double x = 0.0, y = 0.0;
        bool bHaveX = false;
        std::vector<SPoint> tempPoints;    // 存储当前多段线的所有顶点

        // 用于 LINE 实体的临时变量
        double line_x1 = 0, line_y1 = 0, line_x2 = 0, line_y2 = 0;
        int line_state = 0;                 // 0:未开始,1:已读组码10,2:已读组码20,3:已读组码11,4:已读组码21

        while (file.ReadString(strLine))
        {
            strLine.TrimRight(_T("\r\n"));
            if (strLine.IsEmpty())
                continue;

            int nCode = _ttoi(strLine);
            if (!file.ReadString(strLine))
                break;
            strLine.TrimRight(_T("\r\n"));
            CString strValue = strLine;

            // 处理组码 0（实体/段开始/结束标志）
            if (nCode == 0)
            {
                // 结束上一个多段线
                if (bInPolyline && !tempPoints.empty())
                {
                    if (tempPoints.size() >= 2)
                    {
                        if (!tempData.AddLine(tempPoints[0]))
                        {
                            file.Close();
                            return false;
                        }
                        for (size_t k = 1; k < tempPoints.size(); k++)
                            tempData.AddPoint(tempPoints[k], tempData.GetLineSum());
                    }
                    tempPoints.clear();
                }
                bInPolyline = false;

                // 结束 LINE 实体（如果正在解析）
                if (bInLine && line_state >= 4)
                {
                    if (tempData.AddLine(SPoint(line_x1, line_y1, 0)))
                        tempData.AddPoint(SPoint(line_x2, line_y2, 0), tempData.GetLineSum());
                    line_state = 0;
                }
                bInLine = false;

                // 处理段开始/结束
                if (strValue == _T("SECTION"))
                {
                    if (file.ReadString(strLine))
                    {
                        strLine.TrimRight(_T("\r\n"));
                        int secCode = _ttoi(strLine);
                        if (secCode == 2 && file.ReadString(strLine))
                        {
                            strLine.TrimRight(_T("\r\n"));
                            CString secName = strLine;
                            if (secName == _T("ENTITIES"))
                                bInEntities = true;
                        }
                    }
                    continue;
                }
                else if (strValue == _T("ENDSEC"))
                {
                    bInEntities = false;
                    continue;
                }

                // 在 ENTITIES 段内处理实体开始
                if (bInEntities)
                {
                    if (strValue == _T("LWPOLYLINE"))
                    {
                        bInPolyline = true;
                        tempPoints.clear();
                    }
                    else if (strValue == _T("LINE"))
                    {
                        bInLine = true;
                        line_state = 0;
                    }
                    // 可扩展其他实体...
                }
                continue;
            }

            // 仅当在 ENTITIES 段内才解析实体数据
            if (!bInEntities) continue;

            // 处理 LWPOLYLINE
            if (bInPolyline)
            {
                if (nCode == 10)
                {
                    x = _ttof(strValue);
                    bHaveX = true;
                }
                else if (nCode == 20 && bHaveX)
                {
                    y = _ttof(strValue);
                    bHaveX = false;
                    tempPoints.push_back(SPoint(x, y, 0));
                }
                // 忽略其他组码
            }
            // 处理 LINE
            else if (bInLine)
            {
                // 按顺序记录端点坐标
                if (nCode == 10) { line_x1 = _ttof(strValue); line_state = 1; }
                else if (nCode == 20 && line_state == 1) { line_y1 = _ttof(strValue); line_state = 2; }
                else if (nCode == 11 && line_state == 2) { line_x2 = _ttof(strValue); line_state = 3; }
                else if (nCode == 21 && line_state == 3) { line_y2 = _ttof(strValue); line_state = 4; }
                // 忽略其他组码
            }
        }

        // 文件结束，处理可能残留的实体
        if (bInPolyline && !tempPoints.empty() && tempPoints.size() >= 2)
        {
            if (!tempData.AddLine(tempPoints[0]))
            {
                file.Close();
                return false;
            }
            for (size_t k = 1; k < tempPoints.size(); k++)
                tempData.AddPoint(tempPoints[k], tempData.GetLineSum());
        }
        if (bInLine && line_state >= 4)
        {
            if (tempData.AddLine(SPoint(line_x1, line_y1, 0)))
                tempData.AddPoint(SPoint(line_x2, line_y2, 0), tempData.GetLineSum());
        }

        file.Close();

        // 全部成功，替换当前对象
        *this = tempData;
        GetRect();          // 重新计算包围盒
        ClearSelected();    // 清除可能残留的选择
        return true;
    }
    catch (CFileException* e)
    {
        e->Delete();
        AfxMessageBox(_T("加载DXF文件时发生错误！"));
        return false;
    }
}

bool m_CData::InsertPLT(const CString& filePath)
{
    // 1. 创建临时对象并尝试解析文件
    m_CData tempData;
    if (!tempData.LoadPLT(filePath))   // LoadPLT 会清空 tempData 并加载
        return false;

    // 2. 检查容量
    if (m_LineSum + tempData.GetLineSum() > MAX_LINE_NUM)
    {
        AfxMessageBox(_T("插入失败：线条数量将超出上限。"));
        return false;
    }

    // 3. 将临时对象中的线条转移到当前文档
    for (int i = 0; i < tempData.GetLineSum(); i++)
    {
        m_CLine* pSrcLine = tempData.GetLine(i);
        if (!pSrcLine) continue;

        // 深拷贝线条，并设置正确的线序号
        m_CLine* pNewLine = new m_CLine(*pSrcLine);
        pNewLine->SetLineNum(m_LineSum);   // 当前末尾索引
        m_Line[m_LineSum] = pNewLine;
        m_LineSum++;
    }

    GetRect();   // 重新计算包围盒
    return true;
}

bool m_CData::InsertDXF(const CString& filePath)
{
    m_CData tempData;
    if (!tempData.LoadDXF(filePath))
        return false;

    if (m_LineSum + tempData.GetLineSum() > MAX_LINE_NUM)
    {
        AfxMessageBox(_T("插入失败：线条数量将超出上限。"));
        return false;
    }

    for (int i = 0; i < tempData.GetLineSum(); i++)
    {
        m_CLine* pSrcLine = tempData.GetLine(i);
        if (!pSrcLine) continue;
        m_CLine* pNewLine = new m_CLine(*pSrcLine);
        pNewLine->SetLineNum(m_LineSum);
        m_Line[m_LineSum] = pNewLine;
        m_LineSum++;
    }

    GetRect();
    return true;
}

void m_CData::GetRect()
{//计算图形范围
    m_rcDraw.SetRectEmpty();
    for (int i = 0; i < m_LineSum; i++) {
        for (int j = 0; j < m_Line[i]->GetNum(); j++) {
            if (m_rcDraw.left == m_rcDraw.right && m_rcDraw.top == m_rcDraw.bottom) {
                m_rcDraw.left = m_Line[i]->GetPoint(j).m_x;
                m_rcDraw.right = m_rcDraw.left + 1;
                m_rcDraw.top = m_Line[i]->GetPoint(j).m_y;
                m_rcDraw.bottom = m_rcDraw.top - 1;
            }
            else {
                if (m_Line[i]->GetPoint(j).m_x < m_rcDraw.left)
                    m_rcDraw.left = m_Line[i]->GetPoint(j).m_x;
                if (m_Line[i]->GetPoint(j).m_x > m_rcDraw.right)
                    m_rcDraw.right = m_Line[i]->GetPoint(j).m_x;
                if (m_Line[i]->GetPoint(j).m_y > m_rcDraw.top)
                    m_rcDraw.top = m_Line[i]->GetPoint(j).m_y;
                if (m_Line[i]->GetPoint(j).m_y < m_rcDraw.bottom)
                    m_rcDraw.bottom = m_Line[i]->GetPoint(j).m_y;
            }
        }
    }
}

m_CData::m_CData(const m_CData& other)
{
    m_LineSum = other.m_LineSum;
    for (int i = 0; i < m_LineSum; i++)
        m_Line[i] = new m_CLine(*(other.m_Line[i]));

    // 设置当前曲线指针（需要根据原指针查找对应索引）
    if (other.m_CurLine)
    {
        for (int i = 0; i < m_LineSum; i++)
        {
            if (other.m_Line[i] == other.m_CurLine)
            {
                m_CurLine = m_Line[i];
                break;
            }
        }
    }
    else
        m_CurLine = nullptr;

    m_rcDraw = other.m_rcDraw;
}

m_CData& m_CData::operator=(const m_CData& other)
{
    if (this != &other)
    {
        Clear();  // 先释放当前资源

        m_LineSum = other.m_LineSum;
        for (int i = 0; i < m_LineSum; i++)
            m_Line[i] = new m_CLine(*(other.m_Line[i]));

        if (other.m_CurLine)
        {
            for (int i = 0; i < m_LineSum; i++)
            {
                if (other.m_Line[i] == other.m_CurLine)
                {
                    m_CurLine = m_Line[i];
                    break;
                }
            }
        }
        else
            m_CurLine = nullptr;

        m_rcDraw = other.m_rcDraw;
    }
    return *this;
}

m_CLine* m_CData::GetLine(int idx) const
{
    if (idx >= 0 && idx < m_LineSum)
        return m_Line[idx];
    return nullptr;
}

void m_CData::SetCurLineByIndex(int idx)
{
    if (idx >= 0 && idx < m_LineSum)
    {
        m_CurLine = m_Line[idx];
        // 同时清除当前节点，避免误高亮
        if (m_CurLine)
            m_CurLine->ClearCurPoint();  // 需要先在 m_CLine 中添加 ClearCurPoint()
    }
    else
    {
        m_CurLine = nullptr;
    }
}

bool m_CData::FindNearestPoint(const SPoint& pWorld, double thresholdWorld, int& lineIdx, int& pointIdx)
{
    double minDist = thresholdWorld;
    bool found = false;
    for (int i = 0; i < m_LineSum; i++)
    {
        m_CLine* pLine = m_Line[i];
        int n = pLine->GetNum();
        for (int j = 0; j < n; j++)
        {
            SPoint pt = pLine->GetPoint(j);
            double dx = pt.m_x - pWorld.m_x;
            double dy = pt.m_y - pWorld.m_y;
            double dist = sqrt(dx * dx + dy * dy);
            if (dist < minDist)
            {
                minDist = dist;
                lineIdx = i;
                pointIdx = j;
                found = true;
            }
        }
    }
    return found;
}

// 计算点到线段距离的辅助函数
static double PointToLineDistance(const SPoint& p, const SPoint& a, const SPoint& b, SPoint& foot)
{
    SPoint ab = { b.m_x - a.m_x, b.m_y - a.m_y };
    SPoint ap = { p.m_x - a.m_x, p.m_y - a.m_y };
    double ab2 = ab.m_x * ab.m_x + ab.m_y * ab.m_y;
    if (ab2 == 0.0) // a == b
    {
        foot = a;
        double dx = p.m_x - a.m_x, dy = p.m_y - a.m_y;
        return sqrt(dx * dx + dy * dy);
    }
    double t = (ap.m_x * ab.m_x + ap.m_y * ab.m_y) / ab2;
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    foot.m_x = a.m_x + t * ab.m_x;
    foot.m_y = a.m_y + t * ab.m_y;
    double dx = p.m_x - foot.m_x, dy = p.m_y - foot.m_y;
    return sqrt(dx * dx + dy * dy);
}

bool m_CData::FindNearestLine(const SPoint& pWorld, double thresholdWorld, int& lineIdx, int& segIdx, SPoint& footWorld)
{
    double minDist = thresholdWorld;
    bool found = false;
    for (int i = 0; i < m_LineSum; i++)
    {
        m_CLine* pLine = m_Line[i];
        int n = pLine->GetNum();
        if (n < 2) continue;
        for (int j = 0; j < n - 1; j++)
        {
            SPoint a = pLine->GetPoint(j);
            SPoint b = pLine->GetPoint(j + 1);
            SPoint foot;
            double dist = PointToLineDistance(pWorld, a, b, foot);
            if (dist < minDist)
            {
                minDist = dist;
                lineIdx = i;
                segIdx = j;
                footWorld = foot;
                found = true;
            }
        }
    }
    return found;
}


//选择集
void m_CData::AddSelected(int idx)
{
    if (idx >= 0 && idx < m_LineSum && !IsSelected(idx))
        m_selectedIndices.push_back(idx);
}

void m_CData::RemoveSelected(int idx)
{
    auto it = std::find(m_selectedIndices.begin(), m_selectedIndices.end(), idx);
    if (it != m_selectedIndices.end())
        m_selectedIndices.erase(it);
}

void m_CData::ClearSelected()
{
    m_selectedIndices.clear();
}

bool m_CData::IsSelected(int idx) const
{
    return std::find(m_selectedIndices.begin(), m_selectedIndices.end(), idx) != m_selectedIndices.end();
}

int m_CData::GetSelectedCount() const
{
    return (int)m_selectedIndices.size();
}

std::vector<int> m_CData::GetSelectedIndices() const
{
    return m_selectedIndices;
}

void m_CData::SelectAll()
{
    m_selectedIndices.clear();
    for (int i = 0; i < m_LineSum; i++)
        m_selectedIndices.push_back(i);
}

//复制
void m_CData::CopySelectedToClipboard()
{
    if (m_selectedIndices.empty())
        return;

    m_CData clipData;
    for (int idx : m_selectedIndices)
    {
        m_CLine* pLine = GetLine(idx);
        if (pLine)
        {
            m_CLine* newLine = new m_CLine(*pLine);  // 深拷贝
            newLine->SetLineNum(clipData.m_LineSum);
            clipData.m_Line[clipData.m_LineSum] = newLine;
            clipData.m_LineSum++;
        }
    }
    SetClipboardData(clipData);
}

//剪切
void m_CData::CutSelectedToClipboard()
{
    if (m_selectedIndices.empty())
        return;

    CopySelectedToClipboard();  // 先复制

    // 删除选中的线条（从后往前）
    std::vector<int> indices = m_selectedIndices;
    std::sort(indices.begin(), indices.end(), std::greater<int>());
    for (int idx : indices)
    {
        delete m_Line[idx];
        for (int j = idx; j < m_LineSum - 1; j++)
        {
            m_Line[j] = m_Line[j + 1];
            m_Line[j]->SetLineNum(j);
        }
        m_LineSum--;
    }
    m_selectedIndices.clear();
    m_CurLine = nullptr;
    GetRect();
}

//粘贴
void m_CData::PasteFromClipboard(const SPoint* pTargetWorld /*= nullptr*/)
{
    if (!HasClipboardData())
        return;

    m_CData clipData = GetClipboardData();  // 获取副本
    int totalLines = clipData.GetLineSum();
    if (totalLines == 0) return;

    if (m_LineSum + totalLines > MAX_LINE_NUM)
    {
        AfxMessageBox(_T("粘贴失败：线条数量将超出上限。"));
        return;
    }

    if (pTargetWorld == nullptr)
    {
        // 无目标点：直接粘贴
        for (int i = 0; i < totalLines; i++)
        {
            m_CLine* pLine = clipData.GetLine(i);
            if (pLine)
            {
                m_CLine* newLine = new m_CLine(*pLine);
                newLine->SetLineNum(m_LineSum);
                m_Line[m_LineSum] = newLine;
                m_LineSum++;
            }
        }
    }
    else
    {
        // 有目标点：计算剪贴板图形中心，平移到目标点
        double centerX = 0.0, centerY = 0.0;
        int totalPoints = 0;
        for (int i = 0; i < totalLines; i++)
        {
            m_CLine* pLine = clipData.GetLine(i);
            if (pLine)
            {
                for (int j = 0; j < pLine->GetNum(); j++)
                {
                    SPoint pt = pLine->GetPoint(j);
                    centerX += pt.m_x;
                    centerY += pt.m_y;
                    totalPoints++;
                }
            }
        }
        if (totalPoints > 0)
        {
            centerX /= totalPoints;
            centerY /= totalPoints;
        }

        double dx = pTargetWorld->m_x - centerX;
        double dy = pTargetWorld->m_y - centerY;

        for (int i = 0; i < totalLines; i++)
        {
            m_CLine* pLine = clipData.GetLine(i);
            if (pLine)
            {
                m_CLine* newLine = new m_CLine(*pLine);
                for (int j = 0; j < newLine->GetNum(); j++)
                {
                    SPoint pt = newLine->GetPoint(j);
                    pt.m_x += dx;
                    pt.m_y += dy;
                    newLine->SetPoint(j, pt);
                }
                newLine->SetLineNum(m_LineSum);
                m_Line[m_LineSum] = newLine;
                m_LineSum++;
            }
        }
    }

    // 自动选中新粘贴的线条
    ClearSelected();
    for (int i = m_LineSum - totalLines; i < m_LineSum; i++)
        AddSelected(i);

    GetRect();
}


int m_CData::DeleteSelectedLines()
{
    if (m_selectedIndices.empty())
        return 0;

    // 从后往前删除，避免索引变化
    std::vector<int> indices = m_selectedIndices;
    std::sort(indices.begin(), indices.end(), std::greater<int>());

    int count = 0;
    for (int idx : indices)
    {
        delete m_Line[idx];
        for (int j = idx; j < m_LineSum - 1; j++)
        {
            m_Line[j] = m_Line[j + 1];
            m_Line[j]->SetLineNum(j);
        }
        m_LineSum--;
        count++;
    }

    m_selectedIndices.clear();   // 清空选择集
    m_CurLine = nullptr;         // 当前线条可能已被删除
    GetRect();                   // 更新包围盒
    return count;
}

void m_CData::SetClipboardData(const m_CData& data)
{
    s_clipboardData = data;
}

bool m_CData::HasClipboardData()
{
    return s_clipboardData.GetLineSum() > 0;
}

void m_CData::ClearClipboard()
{
    s_clipboardData.Clear();
}

m_CData m_CData::GetClipboardData()
{
    return s_clipboardData;
}
