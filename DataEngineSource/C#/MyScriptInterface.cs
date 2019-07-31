public abstract class MyScriptInterface
{
    public abstract void OnLoad();
    public abstract void OnPlay();
    public abstract void OnStop();
    public abstract void OnTouch(int action, int id, float x, float y, float pressure, float size);
    public abstract void OnButtons(int action, int id);
    public abstract void Update(float deltaTime);
}
