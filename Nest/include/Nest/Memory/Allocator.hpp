#pragma once

#include <memory>

//#include <pool/pool.hpp>
//
//namespace Memory {
//    class Allocator {
//    public:
//        static void init() {
//            boost::pool<> myPool(sizeof(int)); // Создание объекта boost::pool без привязки к конкретному типу
//        }
//
//        inline static std::shared_ptr<Memory::Allocator> &getAllocator() {
//            return s_allocator;
//        };
//    private:
//        static std::shared_ptr<boost::pool<>> s_allocator;
//    };
//}
