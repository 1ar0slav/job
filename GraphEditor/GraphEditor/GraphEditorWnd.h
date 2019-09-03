#pragma once
class Graph;
class Node;
class GraphEditorWnd : public CWnd
{
    int m_capture; // L or R button was captured
    Node* m_start;
    Node* m_finish;
    Graph* m_graph;
    bool m_drag;
public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnPaint();
//    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
    virtual void PreSubclassWindow();
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};
