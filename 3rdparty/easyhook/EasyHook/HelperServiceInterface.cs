/*
    EasyHook - The reinvention of Windows API hooking
 
    Copyright (C) 2009 Christoph Husse

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    Please visit http://www.codeplex.com/easyhook for more information
    about the project and latest updates.
*/
using System;
using System.Collections.Generic;
using System.Text;
using System.Reflection;
using System.Threading;

namespace EasyHook
{
    #pragma warning disable 1591

    public class HelperServiceInterface : MarshalByRefObject
    {

        public void InjectEx(
                Int32 InHostPID,
                Int32 InTargetPID,
                Int32 InWakeUpTID,
                Int32 InNativeOptions,
                String InLibraryPath_x86,
                String InLibraryPath_x64,
                Boolean InCanBypassWOW64,
                Boolean InCanCreateService,
                params Object[] InPassThruArgs)
        {
            RemoteHooking.InjectEx(
                InHostPID,
                InTargetPID,
                InWakeUpTID,
                InNativeOptions,
                InLibraryPath_x86, 
                InLibraryPath_x64, 
                InCanBypassWOW64,
                InCanCreateService,
                InPassThruArgs);
        }

        public Object ExecuteAsService<TClass>(
                String InMethodName,
                Object[] InParams)
        {
            return typeof(TClass).InvokeMember(InMethodName, BindingFlags.InvokeMethod | BindingFlags.Public |
                BindingFlags.Static, null, null, InParams);
        }

        private class InjectionWait
        {
            public Mutex ThreadLock = new Mutex(false);
            public ManualResetEvent Completion = new ManualResetEvent(false);
            public Exception Error = null;
        }

        private static SortedList<Int32, InjectionWait> InjectionList = new SortedList<Int32, InjectionWait>();

        public static void BeginInjection(Int32 InTargetPID)
        {
            InjectionWait WaitInfo;

            lock (InjectionList)
            {
                if (!InjectionList.TryGetValue(InTargetPID, out WaitInfo))
                {
                    WaitInfo = new InjectionWait();

                    InjectionList.Add(InTargetPID, WaitInfo);
                }
            }

            WaitInfo.ThreadLock.WaitOne();
            WaitInfo.Error = null;
            WaitInfo.Completion.Reset();

            lock (InjectionList)
            {
                if (!InjectionList.ContainsKey(InTargetPID))
                    InjectionList.Add(InTargetPID, WaitInfo);
            }
        }

        public static void EndInjection(Int32 InTargetPID)
        {
            lock (InjectionList)
            {
                InjectionList[InTargetPID].ThreadLock.ReleaseMutex();

                InjectionList.Remove(InTargetPID);
            }
        }

        public static void WaitForInjection(Int32 InTargetPID)
        {
            InjectionWait WaitInfo;

            lock (InjectionList)
            {
                WaitInfo = InjectionList[InTargetPID];
            }

            if (!WaitInfo.Completion.WaitOne(20000, false))
                throw new TimeoutException("Unable to wait for injection completion.");

            if (WaitInfo.Error != null)
                throw WaitInfo.Error;
        }

        public void InjectionException(
            Int32 InClientPID,
            Exception e)
        {
            InjectionWait WaitInfo;

            lock (InjectionList)
            {
                WaitInfo = InjectionList[InClientPID];
            }

            WaitInfo.Error = e;
            WaitInfo.Completion.Set();
        }

        public void InjectionCompleted(Int32 InClientPID)
        {
            InjectionWait WaitInfo;

            lock (InjectionList)
            {
                WaitInfo = InjectionList[InClientPID];
            }

            WaitInfo.Error = null;
            WaitInfo.Completion.Set();
        }

        public void Ping() { }
    }
}
