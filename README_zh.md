# 字体管理组件


## 简介

字体管理组件为系统应用提供了安装、卸载三方字体的能力。

## 目录

字体管理组件源代码目录结构如下所示：

```
/base/global/
├── font_manager            # 字体管理代码仓
│   ├── frameworks          # 字体管理核心代码
│   │   ├── fontmgr         # 字体管理核心代码
│   │   │   ├── include     # 字体管理头文件
│   │   │   ├── src         # 字体管理实现代码
│   │   │   └── test        # 字体管理测试代码
│   ├── interfaces          # 字体管理接口
│   │   └── js/kits         # 字体管理JavaScript接口
│   ├── sa_profile          # 字体管理SA
│   ├── service             # 字体管理服务端、客户端结构
│   │   └── include         # 字体管理服务端、客户端头文件
│   │   └── src             # 字体管理服务端、客户端实现代码
```

## 约束

**语言限制**：JavaScript语言


## 相关仓

全球化子系统

global\_i18n\_standard

**global/font_manager**
