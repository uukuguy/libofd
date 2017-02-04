#ifndef __OFD_PATH_H__
#define __OFD_PATH_H__

#include <memory>
#include <vector>

namespace ofd{

    typedef struct Point{
        double x;
        double y;

        Point() : x(0.0), y(0.0){};
        Point(double _x, double _y): x(_x), y(_y){};

        void Clear(){
            x = y = 0.0;
        }

        void Offset(double dx, double dy){
            x += dx; y += dy;
        }

        bool operator ==(const Point& other) const {
            return x == other.x && y == other.y;
        }

        bool operator !=(const Point& other) const {
            return !(*this == other);
        }

    } *Point_t;

    class OfdSubpath;
    typedef std::shared_ptr<OfdSubpath> OfdSubpathPtr;

    // ================ class OfdSubpath ================
    class OfdSubpath{
    public:
        OfdSubpath(const Point &startPoint);
        OfdSubpath(const OfdSubpath* other);
        
        OfdSubpathPtr Clone() const;

        void ClosePath();
        bool IsClosed() const{return m_bClosed;};

        size_t GetNumPoints() const{return m_points.size();};
        const Point& GetFirstPoint() const;
        const Point& GetLastPoint() const; 
        const Point& GetPoint(size_t idx) const;
        void SetPoint(size_t idx, const Point& point);

        void Offset(double dx, double dy);
        void LineTo(const Point& point);
        void CurveTo(const Point& p0, const Point& p1, const Point& p2);

    private:
        std::vector<Point> m_points;
        std::vector<bool> m_curves;
        bool m_bClosed;

    }; // class OfdSubpath

    class OfdPath;
    typedef std::shared_ptr<OfdPath> OfdPathPtr;

    // ================ class OfdPath ================
    class OfdPath{
    public:
        OfdPath();
        ~OfdPath();

        void MoveTo(const Point& startPoint);
        void LineTo(const Point& point);
        void CurveTo(const Point& p0, const Point& p1, const Point& p2);
        void ClosePath();
        void Offset(double dx, double dy);
        void Append(const OfdPath& otherPath);

        size_t GetNumSubpathes() const {return m_subPathes.size();};
        OfdSubpathPtr GetLastSubpath() const;

    private:
        bool m_bJustMoved;
        Point m_startPoint;
        std::vector<OfdSubpathPtr> m_subPathes;

    }; // class OfdPath

}; // namespace ofd

#endif // __OFD_PATH_H__
