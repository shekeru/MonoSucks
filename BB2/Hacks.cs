using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using UnityEngine;

namespace BB2
{
	public static class Loader
	{
		public static void Init()
		{
			var gameObject = new UnityEngine.GameObject();
			gameObject.AddComponent<Hacks>();
			//System.Console.WriteLine(gameObject);
			UnityEngine.Object.DontDestroyOnLoad(gameObject);
			MessageBox.Show("Injected!", "Hello World!");
		}
		public static void Unload()
		{
		}
	}

	public class Hacks : MonoBehaviour
	{
		void Update()
		{
		}

		void Start()
		{

		}
		void OnGUI()
		{
			GUI.Label(new Rect(0, 0, 40, 40), "fuck me");
		}
	}
}
