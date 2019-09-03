#include "pch.h"
#include "GraphEditorWnd.h"
#include <utility>
#include <functional>

const int Radius = 10; // Radius of attraction

// Square of Hypot
int SqHypot(int x, int y) {
    return x * x + y * y;
}

// Checks if point belongs to segment
bool PtOnSegemnt(const CPoint& point, const CPoint& start, const CPoint& finish) {
    if (start == finish)
        return 0;
    int a = finish.y - start.y;
    int b = finish.x - start.x;
    auto z = sqrt(SqHypot(a, b));
    auto sum = sqrt(SqHypot(point.x - start.x, point.y - start.y)) + sqrt(SqHypot(point.x - finish.x, point.y - finish.y));
    return abs(sum - z) < 3;
}

class GraphSystem {
    CDC* m_pDC;
    CPen * m_penNormal;
    CPen * m_penHover;
public:
    explicit GraphSystem(CDC* pDC, const CRect & rc) : m_pDC(pDC) {
        pDC->FillSolidRect(&rc, RGB(240,250,250));
        m_penNormal = new CPen(PS_SOLID | PS_COSMETIC, 2, RGB(0, 100, 200));
        m_penHover = new CPen(PS_SOLID | PS_COSMETIC, 2, RGB(200, 0, 0));
    }
    ~GraphSystem() {
        delete m_penNormal;
        delete m_penHover;
    }
    void DrawNode(const CPoint& point, const CString& text, bool hover = false) const {
        int nSaveDC = m_pDC->SaveDC();
        if(hover)
            m_pDC->SelectObject(m_penHover);
        else
            m_pDC->SelectObject(m_penNormal);
        CRect rc(point, CSize(0, 0));
        rc.InflateRect(10, 10);
        m_pDC->Ellipse(&rc);
        m_pDC->SetBkMode(TRANSPARENT);
        m_pDC->DrawText(text, &rc, DT_SINGLELINE | DT_CENTER);
        m_pDC->RestoreDC(nSaveDC);
    }
    void DrawEdge(CPoint start, CPoint finish, bool exact = false, bool hover = false) const {
        int nSaveDC = m_pDC->SaveDC();
        if (hover)
            m_pDC->SelectObject(m_penHover);
        else
            m_pDC->SelectObject(m_penNormal);
        SegmentCircleIntersect(finish, start, 10);
        m_pDC->MoveTo(start);
        if(!exact)
            SegmentCircleIntersect(start, finish, 10 + 3);
        m_pDC->LineTo(finish);
        // TODO: Draw arrow
        CRect rc(finish, CSize(0, 0));
        rc.InflateRect(3, 3);
        m_pDC->Ellipse(&rc);
        m_pDC->RestoreDC(nSaveDC);
    }
private:
    /// Calculates intersection point of segment and circle with center center in finish and Radius
    void SegmentCircleIntersect(const CPoint& start, CPoint& finish, int r) const {
        int dx = finish.x - start.x;
        int dy = finish.y - start.y;
        if (dx != 0) { // not vertical line
            int q = (int)sqrt(SqHypot(dx, dy));
            finish.x -= r * dx / q;
            finish.y -= r * dy / q;
        }
        else if (dy > 0) {
            finish.y -= r;
        }
        else {
            finish.y += r;
        }
    }
};

class Node {
    friend class Graph;
    CPoint m_point;
    CString m_text;
    std::list<Node*> m_children;
public:
    explicit Node(CPoint&& point, CString text) {
        m_point = point;
        m_text = text;
    }
    const CPoint& Point() const {
        return m_point;
    }
protected:
    /// Adds node graph avoiding links to self and to each other
    bool AddNode(Node* node) {
        // do not allow to make link to self node
        if (this == node)
            return false;
        // do not allow to make link twice
        auto it = std::find(m_children.begin(), m_children.end(), node);
        if (it != m_children.end())
            return false;
        // do not allow to make links to each other
        it = std::find(node->m_children.begin(), node->m_children.end(), this);
        if (it != node->m_children.end())
            return false;
        // here can disable loops on graph
        m_children.push_back(node);
        return true;
    }
    /// Deletes node from node children
    void DeleteNode(Node* node) {
        auto it = std::find(m_children.begin(), m_children.end(), node);
        if (it != m_children.end())
            m_children.erase(it);
    }
    void DrawNode(const GraphSystem& gc, bool hover = false) const {
        gc.DrawNode(m_point, m_text, hover);
    }
    void DrawEdges(const GraphSystem& gc, Node* hoverFinish = nullptr) const {
        for (auto node : m_children) {
            if(hoverFinish && hoverFinish == node)
                gc.DrawEdge(m_point, node->m_point, false, true);
            else
                gc.DrawEdge(m_point, node->m_point);
        }
    }
    bool HitTest(const CPoint& point) {
        CSize sz = m_point - point;
        return SqHypot(sz.cx, sz.cy) < Radius * Radius;
    }
};

class Graph {
    std::vector<Node*> m_nodes;
    int m_current;
    Node* m_hoverNode;
    Node* m_hoverEdge;
public:
    explicit Graph(): m_current(0), m_hoverNode(nullptr), m_hoverEdge(nullptr) {
        // self test
        AddNode(CPoint(100, 100));
        AddNode(CPoint(200, 100));
        AddNode(CPoint(150, 150));
        auto node = Vertex(CPoint(100, 100));
        ASSERT(node);
        auto ch1 = Vertex(CPoint(200, 100));
        ASSERT(ch1);
        auto ch2 = Vertex(CPoint(150, 150));
        ASSERT(ch2);
        AddEdge(node, ch1);
        AddEdge(ch1, ch2);
        AddEdge(ch2, node);
        auto edge = Edge(CPoint(150, 100));
        ASSERT(edge.first == node);
        ASSERT(edge.second == ch1);
        m_nodes.clear();
        m_current = 0;
    }
    /// Draw scene
    void DrawScene(GraphSystem& gc) {
        for (auto node : m_nodes) {
            if(m_hoverNode && !m_hoverEdge && m_hoverNode == node)
                node->DrawNode(gc, true);
            else
                node->DrawNode(gc);
        }
        for (auto node : m_nodes) {
            if (m_hoverEdge && m_hoverNode && m_hoverNode == node)
                node->DrawEdges(gc, m_hoverEdge);
            else
                node->DrawEdges(gc);
        }
    }
    /// Find node which attracts cursor
    Node* Vertex(const CPoint& point) {
        auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [point](auto node) { return node->HitTest(point); });
        return it == m_nodes.end() ? nullptr : *it;
    }
    std::pair<Node*, Node*> Edge(const CPoint & point) {
        for (auto start : m_nodes) {
            auto it = std::find_if(start->m_children.begin(), start->m_children.end(), [start, point](auto finish) {
                return PtOnSegemnt(point, start->m_point, finish->m_point);
                });
            if (it != start->m_children.end())
                return std::make_pair(start, *it);
        }
        return std::make_pair(nullptr, nullptr);
    }
    void AddNode(CPoint&& point) {
        CString str;
        str.Format(_T("%d"), ++m_current);
        m_nodes.push_back(new Node(std::move(point), std::move(str)));
    }
    void AddEdge(Node* start, Node* finish) {
        start->AddNode(finish);
    }
    void DelNode(Node* node) {
        auto n = m_nodes.end();
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            auto p = *it;
            if (node == p) {
                n = it;
            }
            else {
                auto child = std::find(p->m_children.begin(), p->m_children.end(), node);
                if (child != p->m_children.end())
                    p->m_children.erase(child);
            }
        }
        if (n != m_nodes.end()) {
            m_hoverNode = m_hoverEdge = nullptr;
            m_nodes.erase(n);
        }
    }
    void DelEdge(Node* start, Node * finish) {
        auto it = std::find(start->m_children.begin(), start->m_children.end(), finish);
        if (it != start->m_children.end())
            start->m_children.erase(it);
    }
    /// Check which object is under the mouse and return true if changed
    bool Hover(const CPoint& point) {
        // check if same vertex
        if (m_hoverNode && !m_hoverEdge && m_hoverNode->HitTest(point))
            return false;
        // check if any vertex
        auto node = Vertex(point);
        if (node) {
            m_hoverNode = node;
            m_hoverEdge = nullptr;
            return true;
        }
        // check if any edge
        auto edge = Edge(point);
        if (edge.first) {
            if (m_hoverNode == edge.first && m_hoverEdge == edge.second)
                return false;
            m_hoverNode = edge.first;
            m_hoverEdge = edge.second;
            return true;
        }
        if (!m_hoverNode && !m_hoverEdge)
            return false;
        m_hoverNode = m_hoverEdge = nullptr;
        return true;
    }
    void MoveNode(Node * node, const CPoint& point) {
        node->m_point = point;
    }
};

BEGIN_MESSAGE_MAP(GraphEditorWnd, CWnd)
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_PAINT()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

enum Operation {
    OP_NEW_EDGE,
    OP_MOVE_VERTEX,
};

void GraphEditorWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (GetCapture() == this)
        return;
    m_start = m_graph->Vertex(point);
    if (!m_start) // put new point
    {
        m_graph->AddNode(std::move(point));
        this->RedrawWindow();
        return;
    }
    else // drag-and-drop point
    {
        m_capture = MK_LBUTTON;
        SetCapture();
    }
}

void GraphEditorWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (GetCapture() == this && m_capture == MK_LBUTTON) {
        ReleaseCapture();
        RedrawWindow();
    }
}

void GraphEditorWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
    if (GetCapture() == this)
        return;
    m_start = m_graph->Vertex(point);
    if (m_start) {
        m_capture = MK_RBUTTON;
        m_drag = false;
        SetCapture();
        RedrawWindow();
        return;
    }
    auto edge = m_graph->Edge(point);
    if (edge.first && edge.second) {
        m_graph->DelEdge(edge.first, edge.second);
        RedrawWindow();
    }
}

void GraphEditorWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
    if (GetCapture() == this && m_capture == MK_RBUTTON) {
        ReleaseCapture();
        // new edge
        if (m_drag) {
            auto node = m_graph->Vertex(point);
            if (m_start && node)
                m_graph->AddEdge(m_start, node);
        }
        // delete vertex
        else if(m_start) {
            m_graph->DelNode(m_start);
        }
        RedrawWindow();
    }
}


void GraphEditorWnd::OnMouseMove(UINT nFlags, CPoint point)
{
    if (GetCapture() == this && m_start) {
        if (!m_drag) {
            CSize sz = point - m_start->Point();
            m_drag = (sz.cx * sz.cx + sz.cy * sz.cy) > 3 * 3;
        }
        if (m_drag && m_capture == MK_LBUTTON) {
            m_graph->MoveNode(m_start, point);
        }
        RedrawWindow();
    }
    else if (m_graph->Hover(point)) {
        RedrawWindow();
    }
}


void GraphEditorWnd::OnPaint()
{
    CRect rc;
    GetClientRect(&rc);
    CPaintDC dc(this);
    dc.IntersectClipRect(&rc);
    GraphSystem gs(&dc, rc);
    m_graph->DrawScene(gs);
    if (GetCapture() == this) {
        if (m_start && m_capture == MK_RBUTTON) {
            if (m_drag) {
                CPoint point;
                GetCursorPos(&point);
                ScreenToClient(&point);
                gs.DrawEdge(m_start->Point(), point, true);
            }
        }
    }
}


void GraphEditorWnd::PreSubclassWindow()
{
    m_capture = 0;
    m_start = nullptr;
    m_finish = nullptr;
    m_graph = new Graph();
    CWnd::PreSubclassWindow();
}


void foo(int p1, int p2, int p3, int p4, int p5) {

}

void goo() {
    std::bind(foo, 1, 2, 3, 4, 5);
}