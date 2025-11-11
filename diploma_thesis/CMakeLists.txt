# CMakeList.txt: проект CMake для diploma_thesis; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.20)

set (CMAKE_CXX_STANDARD 20)
# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (file_server)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OPENSSL_ROOT_DIR D:/My_Programs/C++/openssl)
set(BOOST_ROOT D:/My_Programs/C++/boost_1_89_0)
set(PostgreSQL_ROOT D:/Programs/PostgreSQL/bin) 
#set(BOOST_INCLUDEDIR ${BOOST_ROOT}/include)
#set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib)

#Если они не найдены, CMake завершит процесс с ошибкой.
find_package(Boost REQUIRED COMPONENTS)
find_package(PostgreSQL REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_ROOT_DIR})

add_subdirectory(D:/My_Programs/C++/libpqxx pqxx-build)
#Добавляет директории с заголовочными файлами Boost в список путей для поиска заголовков.
include_directories(${Boost_INCLUDE_DIRS})

#Создает исполняемый файл file_server, используя указанные исходные файлы
add_executable(file_server
        diploma_thesis.cpp
        spider.hpp
        spider.cpp 
        http_client_async_ssl.cpp 
        http_client_async.cpp
        html_parser.hpp
        html_parser.cpp
        data_base.hpp
        data_base.cpp
        ini_parser.hpp
        ini_parser.cpp
        parser_exceptions.hpp
        root_certificates.hpp
        tasks_queue.hpp
        tasks_queue.cpp)

#target_link_libraries(file_server PRIVATE ${OPENSSL_LIBRARIES})
target_compile_features(file_server PRIVATE cxx_std_17)
#Указывает, что исполняемый файл file_server должен быть связан с библиотеками Boost (filesystem и system)
target_link_libraries(file_server
        PRIVATE
        #Boost::filesystem
        Boost::headers
        pqxx
)

#Устанавливает путь для выходного исполняемого файла, 
#он будет помещен в папку bin внутри директории сборки
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

#Создает директорию для выходного исполняемого файла, если она еще не существует.
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
