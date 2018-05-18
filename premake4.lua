--
-- Building my cone exanple with msys2
--
solution ('Tutorial')
   configurations { 'Release' }
      language 'C++'
    project ('Cone')
            kind 'ConsoleApp'
            links{'glew32', 'glfw3', 'opengl32'}
            files {'*.cpp'}
            buildoptions{'-Wno-write-strings'}
