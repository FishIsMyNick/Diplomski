﻿#pragma checksum "..\..\..\ClientWindow.xaml" "{ff1816ec-aa5e-4d10-87f7-6f4963833460}" "675585757DF7344B3EB3B73F06BDC413F76F30FD"
//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.42000
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

using MotorClient;
using System;
using System.Diagnostics;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Controls.Ribbon;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Effects;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Media.TextFormatting;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Shell;


namespace MotorClient {
    
    
    /// <summary>
    /// ClientWindow
    /// </summary>
    public partial class ClientWindow : System.Windows.Window, System.Windows.Markup.IComponentConnector {
        
        
        #line 22 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button btn_quit;
        
        #line default
        #line hidden
        
        
        #line 39 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button btn_cmd1;
        
        #line default
        #line hidden
        
        
        #line 46 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox tb_speed_cw;
        
        #line default
        #line hidden
        
        
        #line 48 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox tb_duration_cw;
        
        #line default
        #line hidden
        
        
        #line 49 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button btn_addCwCmd;
        
        #line default
        #line hidden
        
        
        #line 53 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button btn_cmd2;
        
        #line default
        #line hidden
        
        
        #line 61 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox tb_speed_ccw;
        
        #line default
        #line hidden
        
        
        #line 63 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.TextBox tb_duration_ccw;
        
        #line default
        #line hidden
        
        
        #line 64 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button btn_addCcwCmd;
        
        #line default
        #line hidden
        
        
        #line 73 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.ListView lv_komande;
        
        #line default
        #line hidden
        
        
        #line 87 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button btn_sendCommands;
        
        #line default
        #line hidden
        
        
        #line 88 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.Button btn_removeCommand;
        
        #line default
        #line hidden
        
        
        #line 93 "..\..\..\ClientWindow.xaml"
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1823:AvoidUnusedPrivateFields")]
        internal System.Windows.Controls.ScrollViewer consoleOutput;
        
        #line default
        #line hidden
        
        private bool _contentLoaded;
        
        /// <summary>
        /// InitializeComponent
        /// </summary>
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.CodeDom.Compiler.GeneratedCodeAttribute("PresentationBuildTasks", "8.0.0.0")]
        public void InitializeComponent() {
            if (_contentLoaded) {
                return;
            }
            _contentLoaded = true;
            System.Uri resourceLocater = new System.Uri("/MotorClient;component/clientwindow.xaml", System.UriKind.Relative);
            
            #line 1 "..\..\..\ClientWindow.xaml"
            System.Windows.Application.LoadComponent(this, resourceLocater);
            
            #line default
            #line hidden
        }
        
        [System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [System.CodeDom.Compiler.GeneratedCodeAttribute("PresentationBuildTasks", "8.0.0.0")]
        [System.ComponentModel.EditorBrowsableAttribute(System.ComponentModel.EditorBrowsableState.Never)]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Design", "CA1033:InterfaceMethodsShouldBeCallableByChildTypes")]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity")]
        [System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1800:DoNotCastUnnecessarily")]
        void System.Windows.Markup.IComponentConnector.Connect(int connectionId, object target) {
            switch (connectionId)
            {
            case 1:
            
            #line 11 "..\..\..\ClientWindow.xaml"
            ((MotorClient.ClientWindow)(target)).Closing += new System.ComponentModel.CancelEventHandler(this.Window_Closing);
            
            #line default
            #line hidden
            return;
            case 2:
            this.btn_quit = ((System.Windows.Controls.Button)(target));
            
            #line 22 "..\..\..\ClientWindow.xaml"
            this.btn_quit.Click += new System.Windows.RoutedEventHandler(this.btn_quit_Click);
            
            #line default
            #line hidden
            return;
            case 3:
            this.btn_cmd1 = ((System.Windows.Controls.Button)(target));
            
            #line 43 "..\..\..\ClientWindow.xaml"
            this.btn_cmd1.Click += new System.Windows.RoutedEventHandler(this.btn_cmd1_Click);
            
            #line default
            #line hidden
            return;
            case 4:
            this.tb_speed_cw = ((System.Windows.Controls.TextBox)(target));
            return;
            case 5:
            this.tb_duration_cw = ((System.Windows.Controls.TextBox)(target));
            return;
            case 6:
            this.btn_addCwCmd = ((System.Windows.Controls.Button)(target));
            
            #line 49 "..\..\..\ClientWindow.xaml"
            this.btn_addCwCmd.Click += new System.Windows.RoutedEventHandler(this.btn_addCwCmd_Click);
            
            #line default
            #line hidden
            return;
            case 7:
            this.btn_cmd2 = ((System.Windows.Controls.Button)(target));
            
            #line 58 "..\..\..\ClientWindow.xaml"
            this.btn_cmd2.Click += new System.Windows.RoutedEventHandler(this.btn_cmd2_Click);
            
            #line default
            #line hidden
            return;
            case 8:
            this.tb_speed_ccw = ((System.Windows.Controls.TextBox)(target));
            return;
            case 9:
            this.tb_duration_ccw = ((System.Windows.Controls.TextBox)(target));
            return;
            case 10:
            this.btn_addCcwCmd = ((System.Windows.Controls.Button)(target));
            
            #line 64 "..\..\..\ClientWindow.xaml"
            this.btn_addCcwCmd.Click += new System.Windows.RoutedEventHandler(this.btn_addCcwCmd_Click);
            
            #line default
            #line hidden
            return;
            case 11:
            this.lv_komande = ((System.Windows.Controls.ListView)(target));
            return;
            case 12:
            this.btn_sendCommands = ((System.Windows.Controls.Button)(target));
            
            #line 87 "..\..\..\ClientWindow.xaml"
            this.btn_sendCommands.Click += new System.Windows.RoutedEventHandler(this.btn_sendCommands_Click);
            
            #line default
            #line hidden
            return;
            case 13:
            this.btn_removeCommand = ((System.Windows.Controls.Button)(target));
            
            #line 88 "..\..\..\ClientWindow.xaml"
            this.btn_removeCommand.Click += new System.Windows.RoutedEventHandler(this.btn_removeCommand_Click);
            
            #line default
            #line hidden
            return;
            case 14:
            this.consoleOutput = ((System.Windows.Controls.ScrollViewer)(target));
            return;
            }
            this._contentLoaded = true;
        }
    }
}

