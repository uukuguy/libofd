#include <assert.h>
#include "ofd/Path.h"
#include "utils/logger.h"

using namespace ofd;

// **************** class Subpath ****************

Subpath::Subpath(const Point &startPoint) :
    m_bClosed(false){
    m_points.push_back(startPoint);
}

Subpath::Subpath(const Subpath *other) :
    m_bClosed(false){
    std::copy(other->m_points.begin(), other->m_points.end(), m_points.begin());
}

SubpathPtr Subpath::Clone() const{
    return std::make_shared<Subpath>(this);
}

const Point& Subpath::GetFirstPoint() const{
    assert(m_points.size() > 0);
    return m_points[0];
}

const Point& Subpath::GetLastPoint() const{
    assert(m_points.size() > 0);
    return m_points[m_points.size() - 1];
}

const Point& Subpath::GetPoint(size_t idx) const{
    assert(idx < m_points.size());
    return m_points[idx];
}

void Subpath::SetPoint(size_t idx, const Point& point){
    assert(idx < m_points.size());
    m_points[idx] = point;
}

void Subpath::ClosePath(){
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

void Subpath::Offset(double dx, double dy){
    std::for_each(m_points.begin(), m_points.end(), 
            [=](Point& point){
                point.Offset(dx, dy);
            });
}

void Subpath::LineTo(const Point& point){
    m_points.push_back(point);
    m_curves.push_back(false);
}

void Subpath::CurveTo(const Point& p0, const Point& p1, const Point& p2){
    m_points.push_back(p0);
    m_points.push_back(p1);
    m_points.push_back(p2);
    m_curves.push_back(true);
    m_curves.push_back(true);
    m_curves.push_back(false);
}

// **************** class Path ****************

Path::Path() :
    m_bJustMoved(false){
}

Path::~Path(){
}

// ======== Path::GetLastSubpath() ========
SubpathPtr Path::GetLastSubpath() const{
    size_t numSubpaths = GetNumSubpaths();
    if ( numSubpaths > 0 ){
        return m_subpaths[numSubpaths - 1];
    } else {
        return nullptr;
    }
}

// ======== Path::MoveTo() ========
void Path::MoveTo(const Point& startPoint){
    m_bJustMoved = true;
    m_startPoint = startPoint;
}

// ======== Path::LineTo() ========
void Path::LineTo(const Point& point){

    if ( m_bJustMoved ){
        SubpathPtr subPath = std::make_shared<Subpath>(m_startPoint);
        m_subpaths.push_back(subPath);
    } else {
        SubpathPtr lastSubpath = GetLastSubpath();
        assert(lastSubpath != nullptr );

        if (lastSubpath->IsClosed()){
            const Point& lastPoint = lastSubpath->GetLastPoint();
            SubpathPtr subPath = std::make_shared<Subpath>(lastPoint);
            m_subpaths.push_back(subPath);
        }
    }

    SubpathPtr lastSubpath = GetLastSubpath();
    lastSubpath->LineTo(point);
    m_bJustMoved = false;
}

// ======== Path::CurveTo() ========
void Path::CurveTo(const Point& p0, const Point& p1, const Point& p2){
    if ( m_bJustMoved ){
        SubpathPtr subPath = std::make_shared<Subpath>(m_startPoint);
        m_subpaths.push_back(subPath);
    } else {
        SubpathPtr lastSubpath = GetLastSubpath();
        assert(lastSubpath != nullptr );

        if (lastSubpath->IsClosed()){
            const Point& lastPoint = lastSubpath->GetLastPoint();
            SubpathPtr subPath = std::make_shared<Subpath>(lastPoint);
            m_subpaths.push_back(subPath);
        }
    }

    SubpathPtr lastSubpath = GetLastSubpath();
    lastSubpath->CurveTo(p0, p1, p2);
    m_bJustMoved = false;
}

// ======== Path::ClosePath() ========
void Path::ClosePath(){
    SubpathPtr lastSubpath = GetLastSubpath();
    if (lastSubpath == nullptr ) return;

    if ( m_bJustMoved ){
        SubpathPtr subPath = std::make_shared<Subpath>(m_startPoint);
        m_subpaths.push_back(subPath);
        m_bJustMoved = false;
    }

    lastSubpath = GetLastSubpath();
    lastSubpath->ClosePath();
}

// ======== Path::Offset() ========
void Path::Offset(double dx, double dy){
    std::for_each(m_subpaths.begin(), m_subpaths.end(),
            [=](SubpathPtr subPath){
                subPath->Offset(dx, dy);
            });
}

// ======== Path::Append() ========
void Path::Append(const Path& otherPath){
    std::for_each(otherPath.m_subpaths.begin(), otherPath.m_subpaths.end(),
            [=](SubpathPtr subPath){
                m_subpaths.push_back(subPath->Clone());
            });
    m_bJustMoved = false;
    m_startPoint.Clear();
}
