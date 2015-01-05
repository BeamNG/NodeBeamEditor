
#include "NodeBeam.h"
#include <QDebug>
#include <QString>

using namespace luabridge;

void NodeBeam::RunLUAScript(QString filename)
{
    qDebug() << "Lua!";
    QStringList filenamel = filename.split("/");
    QString msg = "Running Lua script " + filenamel.last();
    emit LUA_log(msg);

    //Intialize Lua state
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    //Register functions to be called from Lua
    getGlobalNamespace(L)
            .beginClass<NodeBeam>("NodeBeam")
            .addConstructor<void (*) (void)>()
            .addFunction("AddNode",&NodeBeam::LuaAddNode)
            .addFunction("AddBeam",&NodeBeam::LuaAddBeam)
            .addFunction("AddComment",&NodeBeam::LuaAddComment)
            .addFunction("Log",&NodeBeam::LuaLog)
            .endClass();

    //Link this nodebeam object to Lua
    push (L, this);
    lua_setglobal (L, "NB");

    //Run Lua script
    luaL_dofile(L, filename.toStdString().c_str());
    lua_pcall(L, 0, 0, 0);

    msg = "Script finished";
    emit LUA_log(msg);

}

//Add message to script log
void NodeBeam::LuaLog(const std::string msg)
{
    emit LUA_log(QString::fromStdString(msg));
}

//Ability to add comments from LUA script into the JBEAM
void NodeBeam::LuaAddComment(const std::string comment)
{
    LuaComments.AddComment(QString::fromStdString(comment));
    LuaComment = 1; //one, until comment is handled in a node or beam adding
}

//Add node from lua
void NodeBeam::LuaAddNode(const std::string name, float locx, float locy, float locz)
{
    TempNode.NodeName = QString::fromStdString(name);
    TempNode.locX = locx;
    TempNode.locY = locy;
    TempNode.locZ = locz;
    TempNode.comments.clear();
    if(LuaComment)
    {
        TempNode.comments = LuaComments;
        LuaComment=0;
        LuaComments.clear();
    }
    AddNode();
}

//Add beam from lua
void NodeBeam::LuaAddBeam(const std::string node1, const std::string node2)
{
    TempBeam.Node1Name = QString::fromStdString(node1);
    TempBeam.Node2Name = QString::fromStdString(node2);
    TempBeam.Node1GlobalID = FindBeamNodeByName(TempBeam.Node1Name);
    TempBeam.Node2GlobalID = FindBeamNodeByName(TempBeam.Node2Name);
    if((TempBeam.Node1GlobalID<0) || (TempBeam.Node2GlobalID<0)) TempBeam.draw = 0;
    else TempBeam.draw = 1;
    TempBeam.comments.clear();
    if(LuaComment)
    {
        TempBeam.comments = LuaComments;
        LuaComment=0;
        LuaComments.clear();
    }
    AddBeamT();
}

/* Comments class */
Comments::Comments()
{

}
Comments::~Comments()
{
    JBEAM_Comments.clear();
}
bool Comments::AddComment(QString Comment)
{
    JBEAM_Comments.append(Comment);
    return true;
}
bool Comments::RemoveComment(int id)
{
    if(id<JBEAM_Comments.size())
    {
        JBEAM_Comments.removeAt(id);
        return true;
    }
    else return false;
}
QString Comments::ReadComment(int id)
{
    if(id<JBEAM_Comments.size())
    {
        return JBEAM_Comments.at(id);
    }
    else
    {
        QString empty = "";
        return empty;
    }
}
int Comments::size()
{
    return JBEAM_Comments.size();
}
void Comments::clear()
{
    JBEAM_Comments.clear();
}

/* Container for temp beams and nodes going to JBEAM widget */
JBEAM_Temp::JBEAM_Temp()
{
    QVector <Node> NewNodes;
    nodes.append(NewNodes);

    QVector <Beam> NewBeams;
    beams.append(NewBeams);

    NodesToAdd=0;
    BeamsToAdd=0;
}

JBEAM_Temp::~JBEAM_Temp()
{
    clear();
}

void JBEAM_Temp::clear()
{
    nodes.clear();
    beams.clear();
    delete_nodes.clear();
    delete_beams.clear();

    QVector <Node> NewNodes;
    nodes.append(NewNodes);

    QVector <Beam> NewBeams;
    beams.append(NewBeams);

    NodesToAdd = 0;
    BeamsToAdd = 0;
}

void JBEAM_Temp::AddNode(Node node)
{
    nodes[nodes.size()-1].append(node);
}

void JBEAM_Temp::AddBeam(Beam beam)
{
    beams[beams.size()-1].append(beam);
}
void JBEAM_Temp::NewNodeGroup()
{
    QVector <Node> NewNodes;
    nodes.append(NewNodes);
}
void JBEAM_Temp::NewBeamGroup()
{
    QVector <Beam> NewBeams;
    beams.append(NewBeams);
}
void JBEAM_Temp::DeleteNode(Node node)
{
    delete_nodes.append(node);
}
void JBEAM_Temp::DeleteBeam(Beam beam)
{
    delete_beams.append(beam);
}
