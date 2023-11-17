#ifndef __OFD_SIGNATURE_H__
#define __OFD_SIGNATURE_H__

#include "Definitions.h"

namespace ofd{

    // ======== class Signatures ========
    // 签名列表
    class Signatures {
    public:
        uint64_t MaxSignId; // 安全标识的最大值。

        class SignatureInfo{
        public:
            std::string ID;      // 签名或签章的标识。
            std::string Type;    // 签名节点的类型。Seal表示安全签章，Sign表示纯数字签名。
            ST_Loc      BaseLoc; // 指向包内的签名描述文件。
        }; // class SignatureInfo
        std::vector<SignatureInfo> SignatureInfos;
    }; // class Signatures

    // ======== class Signature ========
    // 签名文件
    // OFD的数字签名通过对签名描述文件的保护间接实现对OFD原文的保护。
    // 签名结构中的签名信息(SignedInfo)是这一过程中的关键节点，
    // 其中记录了当次数字签名保护的所有文件的二进制摘要信息，
    // 同时将安全算法提供者、签名算法、签名时间和所应用的安全印章等信息也包含在此节点内。
    // 签名描述文件同时包含了签名值将要存放的包内位置，一旦对该文件实施签名保护，
    // 则其对应的包内文件原文以及本次签名对应的附加信息都将不可修改，
    // 从而实现一次数字签名对整个原文内容的保护。
    class Signature {
    public:
        // 签名要保护的原文及本次签名相关的信息
        class SignedInfo{
        public:
            // 创建签名时所用的签章组件提供者信息
            class Provider{
            public:
            }; // class Provider
            Provider Provider;

            std::string SignatureMethod;   // 签名方法，记录安全模块返回的签名算法代码，以便验证时使用。
            ST_TIME     SignatureDateTime; // 签名时间，记录安全模块返回的签名时间，以便验证时使用。

            // 包内文件计算所得的摘要记录列表。
            // 一个受本次签名保护的包内文件对应一个Reference节点。
            class References{
            public:
                std::string CheckMethod; // 摘要方法，视应用场景的不同使用不同的摘要方法。
                                         // 用于各行业应用时，应使用符合该行业安全标准的算法。
                class Reference{
                public:
                    ST_Loc FileRef;         // 指向包内的文件，使用绝对路径。
                    std::string CheckValue; // 对包内文件进行摘要计算，对所得的二进制摘要值进行base64编码。
                }; // class Reference

                std::vector<Reference> References;

            }; // class References
            References References;

            // 本次签名关联的外观（用OFD中的注释来表示），该节点可出现多次。
            class StampAnnot{
            public:
                std::string ID;       // 签章注释的标识。
                ST_RefID    PageRef;  // 引用外观注释所在的页面的标识。
                ST_Box      Boundary; // 签章注释的外观外边框位置，可用于签章注释在页面的定位。
                ST_Box      Clip;     // 签章注释的外观裁剪设置。
            }; // class StampAnnot
            StampAnnot StampAnnot;

            class Seal{
            public:
                ST_Loc BaseLoc; // 指向包内的安全电子印章文件，遵循密码领域的相关规范。
            }; // class Seal
            Seal Seal;
        
        }; // SignedInfo
        SignedInfo SignedInfo;

        ST_Loc SignedValue; // 指向安全签名提供者所返回的针对签名描述文件计算所得的签名值文件。

    }; // class Signaure

}; // namespace ofd

#endif // __OFD_SIGNATURE_H__
