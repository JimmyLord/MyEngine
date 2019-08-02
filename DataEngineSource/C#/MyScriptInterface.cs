public abstract class MyScriptInterface
{
    public abstract void OnLoad();
    public abstract void OnPlay();
    public abstract void OnStop();
    public abstract bool OnTouch(int action, int id, float x, float y, float pressure, float size);
    public abstract bool OnButtons(int action, int id);
    public abstract void Update(float deltaTime);
}
