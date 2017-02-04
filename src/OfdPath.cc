#include <assert.h>
#include "ofd/OfdPath.h"
#include "utils/logger.h"

using namespace ofd;

// **************** class OfdSubpath ****************

OfdSubpath::OfdSubpath(const Point &startPoint) :
    m_bClosed(false){
    m_points.push_back(startPoint);
}

OfdSubpath::OfdSubpath(const OfdSubpath *other) :
    m_bClosed(false){
    std::copy(other->m_points.begin(), other->m_points.end(), m_points.begin());
}

OfdSubpathPtr OfdSubpath::Clone() const{
    return std::make_shared<OfdSubpath>(this);
}

const Point& OfdSubpath::GetFirstPoint() const{
    assert(m_points.size() > 0);
    return m_points[0];
}

const Point& OfdSubpath::GetLastPoint() const{
    assert(m_points.size() > 0);
    return m_points[m_points.size() - 1];
}

const Point& OfdSubpath::GetPoint(size_t idx) const{
    assert(idx < m_points.size());
    return m_points[idx];
}

void OfdSubpath::SetPoint(size_t idx, const Point& point){
    assert(idx < m_points.size());
    m_points[idx] = point;
}

void OfdSubpath::ClosePath(){
    size_t numPoints = GetNumPoints();
    if ( numPoints < 1 ) return;
    Point p0 = m_points[0];
    Point p1 = m_points[numPoints - 1];
    if ( p0 != p1 ){
    //if ( m_points[0] != m_points[numPoints - 1]){
        LineTo(m_points[0]);
    }
    m_bClosed = true;
}

void OfdSubpath::Offset(double dx, double dy){
    std::for_each(m_points.begin(), m_points.end(), 
            [=](Point& point){
                point.Offset(dx, dy);
            });
}

void OfdSubpath::LineTo(const Point& point){
    m_points.push_back(point);
    m_curves.push_back(false);
}

void OfdSubpath::CurveTo(const Point& p0, const Point& p1, const Point& p2){
    m_points.push_back(p0);
    m_points.push_back(p1);
    m_points.push_back(p2);
    m_curves.push_back(true);
    m_curves.push_back(true);
    m_curves.push_back(false);
}

// **************** class OfdPath ****************

OfdPath::OfdPath() :
    m_bJustMoved(false){
}

OfdPath::~OfdPath(){
}

OfdSubpathPtr OfdPath::GetLastSubpath() const{
    size_t numSubpaths = GetNumSubpaths();
    if ( numSubpaths > 0 ){
        return m_subpaths[numSubpaths - 1];
    } else {
        return nullptr;
    }
}

void OfdPath::MoveTo(const Point& startPoint){
    m_bJustMoved = true;
    m_startPoint = startPoint;
}

void OfdPath::LineTo(const Point& point){

    if ( m_bJustMoved ){
        OfdSubpathPtr subPath = std::make_shared<OfdSubpath>(m_startPoint);
        m_subpaths.push_back(subPath);
    } else {
        OfdSubpathPtr lastSubpath = GetLastSubpath();
        assert(lastSubpath != nullptr );

        if (lastSubpath->IsClosed()){
            const Point& lastPoint = lastSubpath->GetLastPoint();
            OfdSubpathPtr subPath = std::make_shared<OfdSubpath>(lastPoint);
            m_subpaths.push_back(subPath);
        }
    }

    OfdSubpathPtr lastSubpath = GetLastSubpath();
    lastSubpath->LineTo(point);
    m_bJustMoved = false;
}

void OfdPath::CurveTo(const Point& p0, const Point& p1, const Point& p2){
    if ( m_bJustMoved ){
        OfdSubpathPtr subPath = std::make_shared<OfdSubpath>(m_startPoint);
        m_subpaths.push_back(subPath);
    } else {
        OfdSubpathPtr lastSubpath = GetLastSubpath();
        assert(lastSubpath != nullptr );

        if (lastSubpath->IsClosed()){
            const Point& lastPoint = lastSubpath->GetLastPoint();
            OfdSubpathPtr subPath = std::make_shared<OfdSubpath>(lastPoint);
            m_subpaths.push_back(subPath);
        }
    }

    OfdSubpathPtr lastSubpath = GetLastSubpath();
    lastSubpath->CurveTo(p0, p1, p2);
    m_bJustMoved = false;
}

void OfdPath::ClosePath(){
    OfdSubpathPtr lastSubpath = GetLastSubpath();
    if (lastSubpath == nullptr ) return;

    if ( m_bJustMoved ){
        OfdSubpathPtr subPath = std::make_shared<OfdSubpath>(m_startPoint);
        m_subpaths.push_back(subPath);
        m_bJustMoved = false;
    }

    lastSubpath = GetLastSubpath();
    lastSubpath->ClosePath();
}

void OfdPath::Offset(double dx, double dy){
    std::for_each(m_subpaths.begin(), m_subpaths.end(),
            [=](OfdSubpathPtr subPath){
                subPath->Offset(dx, dy);
            });
}

void OfdPath::Append(const OfdPath& otherPath){
    std::for_each(otherPath.m_subpaths.begin(), otherPath.m_subpaths.end(),
            [=](OfdSubpathPtr subPath){
                m_subpaths.push_back(subPath->Clone());
            });
    m_bJustMoved = false;
    m_startPoint.Clear();
}

