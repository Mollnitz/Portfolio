using UnityEngine;
using UnityEngine.Events;

public class BPMManager : MonoBehaviour
{
    //constants / magic numbers

    const float BPM = 192;
    const float SecondsInMinute = 60;
    static float BPMRate = 1 / (BPM * SecondsInMinute); // 192 bpm in real time (0.3~ sec)

    //Unity dislikes getters, so I made my own.
    public static float GetBPM()
    {
        return BPMRate;
    }

    //Instance of class.
    public static BPMManager instance;

    public static UnityEvent GlobalTimerEvent;
    
    void Awake()
    {
        //Instance trick, allow only one instance.
        if (instance == null)
        {
            instance = this;
        }
        else
        {
            Destroy(this);
        }

        //Register global time event.
        if (GlobalTimerEvent == null)
        {
            GlobalTimerEvent = new UnityEvent();
        }

        //Start the timer.
        StartCoroutine(GlobalTimer(BPMRate));

    }

    /// <summary>
    /// Sends out a global event on every beat through the GlobalTimerEvent.
    /// </summary>
    /// <param name="inputBPM">beat length in real time (seconds)</param>
    /// <returns></returns>
    IEnumerator GlobalTimer(float inputBPM)
    {
        float startTime = Time.time;
        float nexttick = 0f;

        while (true)
        {
            if ((Time.time - startTime) + (inputBPM) > nexttick)
            {
                nexttick += inputBPM;
                GlobalTimerEvent.Invoke();
            }

            yield return new WaitForEndOfFrame();
        }
    }
}