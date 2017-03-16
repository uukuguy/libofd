#ifndef __OFD__SHADING_H__
#define __OFD__SHADING_H__

#include <memory>
#include <cairo/cairo.h>
#include "ofd/Common.h"
#include "ofd/Color.h"

namespace ofd{

    enum class ShadingType {
        AxialShd = 0, // 径向渐变
        RadialShd,    // 轴向渐变
        GouraudShd,   // 高洛德渐变
        LaGouraudShd, // 网络高洛德渐变
    }; 

    // ======== class BaseShading ========
    // OFD (section 8.3.4) P35，Page.xsd。
    // 基础渐变类
    class Shading {
        public:
            Shading() : Extend(0) {};
            virtual ~Shading(){};

            // =============== Public Attributes ================
        public:
            int           Extend;      // 延长线是否继续绘制渐变？默认值为0，可选。
                                       // 0: 延长线方向不绘制渐变
                                       // 1: 结束(椭圆中心)点往起始(椭圆中心)点延长线方向绘制渐变
                                       // 2: 起始(椭圆中心)点往结束(椭圆中心)点延长线方向绘制渐变
                                       // 3: 两端延长线方向绘制渐变
            ShadingType Type;

            // =============== Public Methods ================
        public:
            virtual cairo_pattern_t *CreateFillPattern(cairo_t *cr){return nullptr;};
            virtual void WriteShadingXML(utils::XMLWriter &writer) const;
            virtual bool ReadShadingXML(utils::XMLElementPtr shadingElement);
            virtual void SetColorStops(const ColorStopArray &colorStops){};

    }; // Shading

    // ======== class AxialShading ========
    // OFD (section 8.3.4.2) P36，Page.xsd。
    // 径向渐变
    class AxialShading : public Shading {
        public:
            AxialShading() : MapType("Direct"), MapUnit(0.0){
                Type = ShadingType::AxialShd;
            }
            virtual ~AxialShading(){};

            // =============== Public Attributes ================
        public:
            Point_t        StartPoint;    // 起始(椭圆)的中心点，可选。
            Point_t        EndPoint;      // 终止(椭圆)的中心点，必选。
            ColorStopArray ColorSegments; // 
            
            std::string   MapType;       // 渐变绘制方式，可取值Direct、Repeat、Reflect，默认值为Direct。
            double        MapUnit;       // 中心点连线上一个渐变区域绘制的长度，
                                       // 当MapType的值不为Direct时出现，默认值为中心点连线长度。

            // =============== Public Methods ================
        public:
            virtual cairo_pattern_t *CreateFillPattern(cairo_t *cr) override;
            virtual void WriteShadingXML(utils::XMLWriter &writer) const override;
            virtual bool ReadShadingXML(utils::XMLElementPtr shadingElement) override;
            virtual void SetColorStops(const ColorStopArray &colorStops) override {ColorSegments = colorStops;};
    }; // class AxialShading

    // ======== class RadialShading ========
    // OFD (section 8.3.4.3) P39，Page.xsd。
    // 轴向渐变
    class RadialShading : public AxialShading {
        public:
            RadialShading() : StartRadius(0.0), EndRadius(0.0), Eccentricity(0.0), Angle(0.0){
                Type = ShadingType::AxialShd;
            }
            virtual ~RadialShading(){};

            // =============== Public Attributes ================
        public:
            double      StartRadius;   // 起始椭圆的长半轴，默认值为0，可选。
            double      EndRadius;     // 终止椭圆的长半轴，必选。
            double      Eccentricity;  // 两个椭圆的离心率，取值[0, 1.0]，默认值为0，此时椭圆退化成圆。
            double      Angle;         // 两个椭圆的倾斜角度，椭圆长轴与短轴正向的夹角，
                                       // 单位为度，默认值为0。

            // =============== Public Methods ================
        public:
            virtual cairo_pattern_t *CreateFillPattern(cairo_t *cr) override;
            virtual void WriteShadingXML(utils::XMLWriter &writer) const override;
            virtual bool ReadShadingXML(utils::XMLElementPtr shadingElement) override;

    }; // class RadialShading


    // ======== class GouraudShading ========
    // OFD (section 8.3.4.4) P45，Page.xsd。
    // 高洛德渐变
    class GouraudShading : public Shading {
        public:
            GouraudShading(){ 
                Type = ShadingType::GouraudShd;
            };
            virtual ~GouraudShading(){};

            // =============== Public Attributes ================
        public:
            ColorArray Colors;

            // =============== Public Methods ================
        public:
            virtual void WriteShadingXML(utils::XMLWriter &writer) const override {};
            bool ReadShadingXML(utils::XMLElementPtr shadingElement) override{return true;};

    }; // GouraudShading


    // ======== class LaGouraudShading ========
    // OFD (section 8.3.4.5) P47，Page.xsd。
    // 网络高洛德渐变
    class LaGouraudShading : public Shading {
        public:
            LaGouraudShading(){ 
                Type = ShadingType::LaGouraudShd;
            };
            virtual ~LaGouraudShading(){};

            // =============== Public Attributes ================
        public:

            // =============== Public Methods ================
        public:
            virtual void WriteShadingXML(utils::XMLWriter &writer) const override {};
            bool ReadShadingXML(utils::XMLElementPtr shadingElement) override{return true;};

    }; // LaGouraudShading


}; // namespace ofd

#endif // __OFD__SHADING_H__
